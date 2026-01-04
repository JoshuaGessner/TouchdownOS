/**
 * @file home_screen.hpp
 * @brief Home screen / watch face for TouchdownOS
 */

#ifndef TOUCHDOWN_SHELL_HOME_SCREEN_HPP
#define TOUCHDOWN_SHELL_HOME_SCREEN_HPP

#include "lvgl.h"
#include "touchdown/core/types.hpp"
#include <string>
#include <memory>

namespace touchdown {
namespace shell {

class HomeScreen {
public:
    HomeScreen();
    ~HomeScreen();
    
    /**
     * @brief Create home screen UI
     */
    void create(lv_obj_t* parent);
    
    /**
     * @brief Update time display
     */
    void update_time();
    
    /**
     * @brief Update battery indicator
     */
    void update_battery(const BatteryInfo& info);
    
    /**
     * @brief Update network indicators
     */
    void update_network(const NetworkInfo& info);
    
    /**
     * @brief Show/hide home screen
     */
    void show();
    void hide();
    
    /**
     * @brief Get root container
     */
    lv_obj_t* get_container() { return container_; }
    
private:
    void create_time_widget();
    void create_date_widget();
    void create_status_indicators();
    void create_quick_actions();
    
    lv_obj_t* container_;
    lv_obj_t* time_label_;
    lv_obj_t* date_label_;
    lv_obj_t* battery_label_;
    lv_obj_t* wifi_icon_;
    lv_obj_t* bt_icon_;
    
    lv_style_t time_style_;
    lv_style_t date_style_;
    lv_style_t status_style_;
};

} // namespace shell
} // namespace touchdown

#endif // TOUCHDOWN_SHELL_HOME_SCREEN_HPP
