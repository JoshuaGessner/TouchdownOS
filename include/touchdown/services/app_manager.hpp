/**
 * @file app_manager.hpp
 * @brief Application lifecycle management service
 */

#ifndef TOUCHDOWN_SERVICES_APP_MANAGER_HPP
#define TOUCHDOWN_SERVICES_APP_MANAGER_HPP

#include "touchdown/app/app.hpp"
#include "touchdown/app/app_registry.hpp"
#include "touchdown/core/types.hpp"
#include <memory>
#include <map>
#include <vector>

namespace touchdown {
namespace services {

enum class AppState {
    RUNNING,
    PAUSED,
    STOPPED
};

struct ManagedApp {
    std::unique_ptr<app::TouchdownApp> instance;
    AppState state;
    uint32_t pid;  // For Python apps
    std::chrono::steady_clock::time_point launch_time;
};

class AppManager {
public:
    AppManager();
    ~AppManager();
    
    /**
     * @brief Initialize the app manager
     */
    bool init();
    
    /**
     * @brief Launch an app by ID
     * @param app_id App identifier
     * @param parent LVGL parent container
     * @return true on success
     */
    bool launch_app(const std::string& app_id, lv_obj_t* parent);
    
    /**
     * @brief Pause an app (save state, stop updates)
     */
    bool pause_app(const std::string& app_id);
    
    /**
     * @brief Resume a paused app
     */
    bool resume_app(const std::string& app_id);
    
    /**
     * @brief Terminate an app
     */
    bool terminate_app(const std::string& app_id);
    
    /**
     * @brief Get currently running app
     */
    app::TouchdownApp* get_active_app() const;
    
    /**
     * @brief Get app state
     */
    AppState get_app_state(const std::string& app_id) const;
    
    /**
     * @brief Check if app is running
     */
    bool is_app_running(const std::string& app_id) const;
    
    /**
     * @brief Update all running apps
     */
    void update(uint32_t delta_ms);
    
    /**
     * @brief Forward touch event to active app
     */
    bool handle_touch(const TouchPoint& point);
    
    /**
     * @brief Forward button event to active app
     */
    bool handle_button(const ButtonEvent& event);
    
    /**
     * @brief Get list of running apps
     */
    std::vector<std::string> get_running_apps() const;
    
    /**
     * @brief Load Python app
     */
    bool launch_python_app(const std::string& app_id, 
                          const std::string& script_path,
                          lv_obj_t* parent);
    
    /**
     * @brief Cleanup all apps
     */
    void cleanup();
    
private:
    bool launch_cpp_app(const std::string& app_id, 
                       const app::AppMetadata& metadata,
                       lv_obj_t* parent);
    
    std::map<std::string, ManagedApp> apps_;
    std::string active_app_id_;
    app::AppRegistry& registry_;
};

} // namespace services
} // namespace touchdown

#endif // TOUCHDOWN_SERVICES_APP_MANAGER_HPP
