/**
 * @file app_registry.cpp
 * @brief App registry implementation
 */

#include "touchdown/app/app_registry.hpp"
#include "touchdown/core/logger.hpp"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace touchdown {
namespace app {

AppRegistry& AppRegistry::instance() {
    static AppRegistry registry;
    return registry;
}

bool AppRegistry::register_app(const std::string& app_id, AppFactory factory) {
    if (factories_.find(app_id) != factories_.end()) {
        TD_LOG_WARNING("AppRegistry", "App already registered: ", app_id);
        return false;
    }
    
    factories_[app_id] = factory;
    TD_LOG_INFO("AppRegistry", "Registered app: ", app_id);
    return true;
}

std::unique_ptr<TouchdownApp> AppRegistry::create_app(const std::string& app_id,
                                                      const AppMetadata& metadata) {
    auto it = factories_.find(app_id);
    if (it == factories_.end()) {
        TD_LOG_ERROR("AppRegistry", "App not registered: ", app_id);
        return nullptr;
    }
    
    try {
        auto app = it->second(metadata);
        TD_LOG_INFO("AppRegistry", "Created app instance: ", app_id);
        return app;
    } catch (const std::exception& e) {
        TD_LOG_ERROR("AppRegistry", "Failed to create app: ", app_id, " - ", e.what());
        return nullptr;
    }
}

std::vector<std::string> AppRegistry::get_app_ids() const {
    std::vector<std::string> ids;
    ids.reserve(factories_.size());
    
    for (const auto& [id, _] : factories_) {
        ids.push_back(id);
    }
    
    return ids;
}

bool AppRegistry::has_app(const std::string& app_id) const {
    return factories_.find(app_id) != factories_.end();
}

AppMetadata AppRegistry::load_metadata(const std::string& manifest_path) {
    AppMetadata metadata;
    
    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        TD_LOG_ERROR("AppRegistry", "Failed to open manifest: ", manifest_path);
        return metadata;
    }
    
    try {
        json j;
        file >> j;
        
        metadata.id = j.value("id", "");
        metadata.name = j.value("name", "");
        metadata.version = j.value("version", "");
        metadata.description = j.value("description", "");
        metadata.icon = j.value("icon", "");
        metadata.type = j.value("type", "cpp");
        metadata.executable = j.value("executable", "");
        
        if (j.contains("permissions")) {
            metadata.permissions = j["permissions"].get<std::vector<std::string>>();
        }
    } catch (const std::exception& e) {
        TD_LOG_ERROR("AppRegistry", "Failed to parse manifest: ", manifest_path, " - ", e.what());
        return metadata;
    }
    
    TD_LOG_INFO("AppRegistry", "Loaded metadata for: ", metadata.id);
    return metadata;
}

std::vector<AppMetadata> AppRegistry::scan_apps(const std::string& apps_dir) {
    std::vector<AppMetadata> apps;
    
    if (!fs::exists(apps_dir)) {
        TD_LOG_WARNING("AppRegistry", "Apps directory not found: ", apps_dir);
        return apps;
    }
    
    for (const auto& entry : fs::directory_iterator(apps_dir)) {
        if (entry.is_directory()) {
            std::string manifest = entry.path().string() + "/manifest.json";
            if (fs::exists(manifest)) {
                AppMetadata metadata = load_metadata(manifest);
                if (!metadata.id.empty()) {
                    apps.push_back(metadata);
                }
            }
        }
    }
    
    TD_LOG_INFO("AppRegistry", "Scanned ", apps.size(), " apps from: ", apps_dir);
    return apps;
}

} // namespace app
} // namespace touchdown
