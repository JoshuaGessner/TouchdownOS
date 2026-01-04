/**
 * @file app_launcher.hpp
 * @brief Circular app launcher for TouchdownOS
 */

#ifndef TOUCHDOWN_SHELL_APP_LAUNCHER_HPP
#define TOUCHDOWN_SHELL_APP_LAUNCHER_HPP

#include "lvgl.h"
#include <string>
#include <vector>
#include <functional>

namespace touchdown {
namespace shell {

struct AppInfo {
    std::string id;
    std::string name;
    std::string icon;  // Symbol or path
    lv_color_t color;
};

class AppLauncher {
public:
    AppLauncher();
    ~AppLauncher();
    
    /**
     * @brief Create app launcher UI
     */
    void create(lv_obj_t* parent);
    
    /**
     * @brief Add app to launcher
     */
    void add_app(const AppInfo& app);
    
    /**
     * @brief Remove app from launcher
     */
    void remove_app(const std::string& app_id);
    
    /**
     * @brief Set app launch callback
     */
    void set_launch_callback(std::function<void(const std::string&)> callback);
    
    /**
     * @brief Show/hide launcher
     */
    void show();
    void hide();
    
    /**
     * @brief Animate launcher appearance
     */
    void animate_show(uint32_t duration_ms = 300);
    void animate_hide(uint32_t duration_ms = 300);
    
    /**
     * @brief Get root container
     */
    lv_obj_t* get_container() { return container_; }
    
private:
    void refresh_layout();
    void create_app_button(const AppInfo& app, int16_t x, int16_t y);
    
    static void on_app_clicked(lv_event_t* e);
    
    lv_obj_t* container_;
    std::vector<AppInfo> apps_;
    std::vector<lv_obj_t*> app_buttons_;
    std::function<void(const std::string&)> launch_callback_;
};

} // namespace shell
} // namespace touchdown

#endif // TOUCHDOWN_SHELL_APP_LAUNCHER_HPP
