/**
 * @file home_screen.cpp
 * @brief Home screen implementation
 */

#include "touchdown/shell/home_screen.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/shell/circular_layout.hpp"
#include "touchdown/core/logger.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace touchdown {
namespace shell {

HomeScreen::HomeScreen()
    : container_(nullptr)
    , time_label_(nullptr)
    , date_label_(nullptr)
    , battery_label_(nullptr)
    , wifi_icon_(nullptr)
    , bt_icon_(nullptr) {
}

HomeScreen::~HomeScreen() {
    if (container_) {
        lv_obj_del(container_);
    }
}

void HomeScreen::create(lv_obj_t* parent) {
    // Create circular container
    container_ = CircularLayout::create_circular_container(parent);
    
    auto& theme = ThemeEngine::instance();
    lv_obj_set_style_bg_color(container_, theme.get_palette().background, 0);
    
    // Create widgets
    create_time_widget();
    create_date_widget();
    create_status_indicators();
    create_quick_actions();
    
    TD_LOG_INFO("HomeScreen", "Home screen created");
}

void HomeScreen::create_time_widget() {
    time_label_ = lv_label_create(container_);
    
    // Style for large time display
    lv_style_init(&time_style_);
    lv_style_set_text_color(&time_style_, ThemeEngine::instance().get_palette().text_primary);
    lv_style_set_text_font(&time_style_, &lv_font_montserrat_28);
    
    lv_obj_add_style(time_label_, &time_style_, 0);
    lv_label_set_text(time_label_, "12:00");
    lv_obj_align(time_label_, LV_ALIGN_CENTER, 0, -20);
}

void HomeScreen::create_date_widget() {
    date_label_ = lv_label_create(container_);
    
    // Style for date display
    lv_style_init(&date_style_);
    lv_style_set_text_color(&date_style_, ThemeEngine::instance().get_palette().text_secondary);
    lv_style_set_text_font(&date_style_, &lv_font_montserrat_14);
    
    lv_obj_add_style(date_label_, &date_style_, 0);
    lv_label_set_text(date_label_, "Mon, Jan 1");
    lv_obj_align(date_label_, LV_ALIGN_CENTER, 0, 10);
}

void HomeScreen::create_status_indicators() {
    auto& theme = ThemeEngine::instance();
    
    lv_style_init(&status_style_);
    lv_style_set_text_color(&status_style_, theme.get_palette().text_secondary);
    lv_style_set_text_font(&status_style_, &lv_font_montserrat_12);
    
    // Battery indicator at top
    battery_label_ = lv_label_create(container_);
    lv_obj_add_style(battery_label_, &status_style_, 0);
    lv_label_set_text(battery_label_, "100%");
    lv_obj_align(battery_label_, LV_ALIGN_TOP_MID, 0, 20);
    
    // WiFi icon
    wifi_icon_ = lv_label_create(container_);
    lv_obj_add_style(wifi_icon_, &status_style_, 0);
    lv_label_set_text(wifi_icon_, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_icon_, LV_ALIGN_BOTTOM_LEFT, 30, -20);
    
    // Bluetooth icon
    bt_icon_ = lv_label_create(container_);
    lv_obj_add_style(bt_icon_, &status_style_, 0);
    lv_label_set_text(bt_icon_, LV_SYMBOL_BLUETOOTH);
    lv_obj_align(bt_icon_, LV_ALIGN_BOTTOM_RIGHT, -30, -20);
}

void HomeScreen::create_quick_actions() {
    // TODO: Add swipeable quick action buttons around the perimeter
}

void HomeScreen::update_time() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    
    // Update time
    char time_buf[16];
    std::strftime(time_buf, sizeof(time_buf), "%H:%M", local);
    lv_label_set_text(time_label_, time_buf);
    
    // Update date
    char date_buf[32];
    std::strftime(date_buf, sizeof(date_buf), "%a, %b %d", local);
    lv_label_set_text(date_label_, date_buf);
}

void HomeScreen::update_battery(const BatteryInfo& info) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", info.percentage);
    
    if (battery_label_) {
        lv_label_set_text(battery_label_, buf);
        
        // Color code based on level
        auto& theme = ThemeEngine::instance();
        lv_color_t color;
        
        if (info.percentage > 50) {
            color = theme.get_palette().success;
        } else if (info.percentage > 20) {
            color = theme.get_palette().warning;
        } else {
            color = theme.get_palette().error;
        }
        
        lv_obj_set_style_text_color(battery_label_, color, 0);
    }
}

void HomeScreen::update_network(const NetworkInfo& info) {
    // Update WiFi icon visibility
    if (wifi_icon_) {
        if (info.wifi_status == NetworkStatus::CONNECTED) {
            lv_obj_clear_flag(wifi_icon_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(wifi_icon_, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // Update Bluetooth icon visibility
    if (bt_icon_) {
        if (info.bluetooth_status == NetworkStatus::CONNECTED) {
            lv_obj_clear_flag(bt_icon_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(bt_icon_, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void HomeScreen::show() {
    if (container_) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void HomeScreen::hide() {
    if (container_) {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

} // namespace shell
} // namespace touchdown
