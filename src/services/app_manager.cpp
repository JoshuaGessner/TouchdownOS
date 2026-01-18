/**
 * @file app_manager.cpp
 * @brief App manager implementation
 */

#include "touchdown/services/app_manager.hpp"
#include "touchdown/core/logger.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

namespace touchdown {
namespace services {

AppManager::AppManager()
    : registry_(app::AppRegistry::instance()) {
}

AppManager::~AppManager() {
    cleanup();
}

bool AppManager::init() {
    TD_LOG_INFO("AppManager", "Initializing app manager");
    
    // Scan for installed apps
    std::string apps_dir = "/usr/share/touchdown/apps";
    auto apps = registry_.scan_apps(apps_dir);
    
    TD_LOG_INFO("AppManager", "Found ", apps.size(), " installed apps");
    
    return true;
}

bool AppManager::launch_app(const std::string& app_id, lv_obj_t* parent) {
    // Check if already running
    if (is_app_running(app_id)) {
        TD_LOG_WARNING("AppManager", "App already running: ", app_id);
        
        // Bring to front
        auto& managed = apps_[app_id];
        if (managed.state == AppState::PAUSED) {
            managed.instance->resume();
            managed.state = AppState::RUNNING;
        }
        managed.instance->show();
        active_app_id_ = app_id;
        return true;
    }
    
    // Load metadata
    std::string manifest_path = "/usr/share/touchdown/apps/" + app_id + "/manifest.json";
    app::AppMetadata metadata = registry_.load_metadata(manifest_path);
    
    if (metadata.id.empty()) {
        TD_LOG_ERROR("AppManager", "Failed to load metadata for: ", app_id);
        return false;
    }
    
    // Check app type (C++ or Python)
    bool is_python = false;  // TODO: Read from metadata
    
    if (is_python) {
        std::string script_path = "/usr/share/touchdown/apps/" + app_id + "/main.py";
        return launch_python_app(app_id, script_path, parent);
    } else {
        return launch_cpp_app(app_id, metadata, parent);
    }
}

bool AppManager::launch_cpp_app(const std::string& app_id,
                               const app::AppMetadata& metadata,
                               lv_obj_t* parent) {
    TD_LOG_INFO("AppManager", "Launching C++ app: ", app_id);
    
    // Create app instance
    auto app = registry_.create_app(app_id, metadata);
    if (!app) {
        TD_LOG_ERROR("AppManager", "Failed to create app: ", app_id);
        return false;
    }
    
    // Initialize app
    if (!app->init(parent)) {
        TD_LOG_ERROR("AppManager", "Failed to initialize app: ", app_id);
        return false;
    }
    
    // Show app
    app->show();
    
    // Store in managed apps
    ManagedApp managed;
    managed.instance = std::move(app);
    managed.state = AppState::RUNNING;
    managed.pid = 0;  // In-process
    managed.launch_time = std::chrono::steady_clock::now();
    
    apps_[app_id] = std::move(managed);
    active_app_id_ = app_id;
    
    TD_LOG_INFO("AppManager", "App launched successfully: ", app_id);
    return true;
}

bool AppManager::launch_python_app(const std::string& app_id,
                                  const std::string& script_path,
                                  lv_obj_t* parent) {
    TD_LOG_INFO("AppManager", "Launching Python app: ", app_id);
    
    // Fork and execute Python script
    pid_t pid = fork();
    
    if (pid < 0) {
        TD_LOG_ERROR("AppManager", "Fork failed for: ", app_id);
        return false;
    }
    
    if (pid == 0) {
        // Child process - execute Python app
        execl("/usr/bin/python3", "python3", script_path.c_str(), nullptr);
        
        // If execl returns, it failed
        TD_LOG_ERROR("AppManager", "Failed to exec Python app: ", app_id);
        exit(1);
    }
    
    // Parent process - track the app
    ManagedApp managed;
    managed.instance = nullptr;  // Python apps are external processes
    managed.state = AppState::RUNNING;
    managed.pid = pid;
    managed.launch_time = std::chrono::steady_clock::now();
    
    apps_[app_id] = std::move(managed);
    active_app_id_ = app_id;
    
    TD_LOG_INFO("AppManager", "Python app launched with PID: ", pid);
    return true;
}

bool AppManager::pause_app(const std::string& app_id) {
    auto it = apps_.find(app_id);
    if (it == apps_.end()) {
        TD_LOG_WARNING("AppManager", "App not found: ", app_id);
        return false;
    }
    
    auto& managed = it->second;
    if (managed.state != AppState::RUNNING) {
        TD_LOG_WARNING("AppManager", "App not running: ", app_id);
        return false;
    }
    
    if (managed.instance) {
        managed.instance->pause();
    } else if (managed.pid > 0) {
        // Send SIGSTOP to Python app
        kill(managed.pid, SIGSTOP);
    }
    
    managed.state = AppState::PAUSED;
    TD_LOG_INFO("AppManager", "App paused: ", app_id);
    return true;
}

bool AppManager::resume_app(const std::string& app_id) {
    auto it = apps_.find(app_id);
    if (it == apps_.end()) {
        TD_LOG_WARNING("AppManager", "App not found: ", app_id);
        return false;
    }
    
    auto& managed = it->second;
    if (managed.state != AppState::PAUSED) {
        TD_LOG_WARNING("AppManager", "App not paused: ", app_id);
        return false;
    }
    
    if (managed.instance) {
        managed.instance->resume();
    } else if (managed.pid > 0) {
        // Send SIGCONT to Python app
        kill(managed.pid, SIGCONT);
    }
    
    managed.state = AppState::RUNNING;
    active_app_id_ = app_id;
    TD_LOG_INFO("AppManager", "App resumed: ", app_id);
    return true;
}

bool AppManager::terminate_app(const std::string& app_id) {
    auto it = apps_.find(app_id);
    if (it == apps_.end()) {
        TD_LOG_WARNING("AppManager", "App not found: ", app_id);
        return false;
    }
    
    auto& managed = it->second;
    
    if (managed.instance) {
        managed.instance->cleanup();
        managed.instance.reset();
    } else if (managed.pid > 0) {
        // Kill Python app
        kill(managed.pid, SIGTERM);
        
        // Wait for process to exit
        int status;
        waitpid(managed.pid, &status, WNOHANG);
    }
    
    apps_.erase(it);
    
    if (active_app_id_ == app_id) {
        active_app_id_.clear();
    }
    
    TD_LOG_INFO("AppManager", "App terminated: ", app_id);
    return true;
}

app::TouchdownApp* AppManager::get_active_app() const {
    if (active_app_id_.empty()) {
        return nullptr;
    }
    
    auto it = apps_.find(active_app_id_);
    if (it == apps_.end()) {
        return nullptr;
    }
    
    return it->second.instance.get();
}

AppState AppManager::get_app_state(const std::string& app_id) const {
    auto it = apps_.find(app_id);
    if (it == apps_.end()) {
        return AppState::STOPPED;
    }
    
    return it->second.state;
}

bool AppManager::is_app_running(const std::string& app_id) const {
    auto it = apps_.find(app_id);
    return it != apps_.end() && it->second.state != AppState::STOPPED;
}

void AppManager::update(uint32_t delta_ms) {
    // Update active app
    if (!active_app_id_.empty()) {
        auto it = apps_.find(active_app_id_);
        if (it != apps_.end() && it->second.instance) {
            if (it->second.state == AppState::RUNNING) {
                it->second.instance->update(delta_ms);
            }
        }
    }
    
    // Check for dead Python processes
    for (auto it = apps_.begin(); it != apps_.end();) {
        if (it->second.pid > 0) {
            int status;
            pid_t result = waitpid(it->second.pid, &status, WNOHANG);
            
            if (result > 0) {
                // Process exited
                TD_LOG_INFO("AppManager", "Python app exited: ", it->first);
                it = apps_.erase(it);
                continue;
            }
        }
        ++it;
    }
}

bool AppManager::handle_touch(const TouchPoint& point) {
    auto* app = get_active_app();
    if (app) {
        return app->on_touch(point);
    }
    return false;
}

bool AppManager::handle_button(const ButtonEvent& event) {
    auto* app = get_active_app();
    if (app) {
        return app->on_button(event);
    }
    return false;
}

std::vector<std::string> AppManager::get_running_apps() const {
    std::vector<std::string> running;
    
    for (const auto& [app_id, managed] : apps_) {
        if (managed.state != AppState::STOPPED) {
            running.push_back(app_id);
        }
    }
    
    return running;
}

void AppManager::cleanup() {
    TD_LOG_INFO("AppManager", "Cleaning up all apps");
    
    // Terminate all apps
    std::vector<std::string> app_ids;
    for (const auto& [app_id, _] : apps_) {
        app_ids.push_back(app_id);
    }
    
    for (const auto& app_id : app_ids) {
        terminate_app(app_id);
    }
    
    apps_.clear();
    active_app_id_.clear();
}

} // namespace services
} // namespace touchdown
