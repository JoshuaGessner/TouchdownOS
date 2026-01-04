/**
 * @file settings_app.cpp
 * @brief Settings app implementation
 */

#include "touchdown/apps/settings_app.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/shell/circular_layout.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/config.hpp"

namespace touchdown {
namespace apps {

SettingsApp::SettingsApp(const app::AppMetadata& metadata)
    : TouchdownApp(metadata)
    , list_(nullptr) {
}

SettingsApp::~SettingsApp() {
}

bool SettingsApp::init(lv_obj_t* parent) {
    LOG_INFO("SettingsApp", "Initializing settings app");
    
    // Create container
    create_container(parent);
    
    auto& theme = shell::ThemeEngine::instance();
    lv_obj_set_style_bg_color(container_, theme.get_palette().background, 0);
    
    create_ui();
    
    return true;
}

void SettingsApp::create_ui() {
    auto& theme = shell::ThemeEngine::instance();
    
    // Title
    lv_obj_t* title = lv_label_create(container_);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, theme.get_palette().text_primary, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create scrollable list
    list_ = lv_list_create(container_);
    lv_obj_set_size(list_, 200, 160);
    lv_obj_align(list_, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_color(list_, theme.get_palette().surface, 0);
    lv_obj_set_style_border_width(list_, 0, 0);
    lv_obj_set_style_radius(list_, 12, 0);
    
    // Setting items
    std::vector<SettingItem> settings = {
        {
            "Theme",
            "Dark",
            [this]() { on_theme_toggle(); }
        },
        {
            "Brightness",
            "100%",
            [this]() { on_brightness_adjust(); }
        },
        {
            "About",
            "v0.1.0",
            [this]() { on_about(); }
        }
    };
    
    for (size_t i = 0; i < settings.size(); i++) {
        create_setting_item(settings[i], i);
    }
}

void SettingsApp::create_setting_item(const SettingItem& item, int index) {
    lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_SETTINGS, item.label.c_str());
    
    // Store callback in user data
    lv_obj_set_user_data(btn, (void*)&item.on_click);
    
    // Add click handler
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        auto* callback = (std::function<void()>*)lv_obj_get_user_data(lv_event_get_target(e));
        if (callback) {
            (*callback)();
        }
    }, LV_EVENT_CLICKED, nullptr);
    
    items_.push_back(btn);
}

void SettingsApp::on_theme_toggle() {
    LOG_INFO("SettingsApp", "Toggle theme");
    
    auto& theme = shell::ThemeEngine::instance();
    auto current = theme.get_mode();
    
    if (current == shell::ThemeMode::DARK) {
        theme.animate_theme_change(shell::ThemeMode::LIGHT);
        Config::instance().set_string("theme.mode", "light");
    } else {
        theme.animate_theme_change(shell::ThemeMode::DARK);
        Config::instance().set_string("theme.mode", "dark");
    }
    
    Config::instance().save("/etc/touchdown/shell.conf");
}

void SettingsApp::on_brightness_adjust() {
    LOG_INFO("SettingsApp", "Adjust brightness");
    
    // TODO: Create brightness slider popup
    // For now, just toggle between 50% and 100%
    int current = Config::instance().get_int("display.brightness", 255);
    int new_brightness = (current == 255) ? 128 : 255;
    
    Config::instance().set_int("display.brightness", new_brightness);
    Config::instance().save("/etc/touchdown/shell.conf");
    
    // Send D-Bus message to power service to update brightness
}

void SettingsApp::on_about() {
    LOG_INFO("SettingsApp", "Show about");
    
    // Create modal dialog
    lv_obj_t* dialog = lv_obj_create(container_);
    lv_obj_set_size(dialog, 180, 120);
    lv_obj_center(dialog);
    
    auto& theme = shell::ThemeEngine::instance();
    lv_obj_set_style_bg_color(dialog, theme.get_palette().surface, 0);
    lv_obj_set_style_radius(dialog, 16, 0);
    
    lv_obj_t* label = lv_label_create(dialog);
    lv_label_set_text(label, 
        "TouchdownOS\n"
        "Version 0.1.0\n\n"
        "Custom LVGL OS\n"
        "for wearables");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
}

void SettingsApp::show() {
    visible_ = true;
    if (container_) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void SettingsApp::hide() {
    visible_ = false;
    if (container_) {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void SettingsApp::cleanup() {
    LOG_INFO("SettingsApp", "Cleanup");
    items_.clear();
}

bool SettingsApp::on_touch(const TouchPoint& point) {
    // LVGL handles touch automatically for list items
    return false;
}

bool SettingsApp::on_button(const ButtonEvent& event) {
    if (event.type == ButtonEventType::SINGLE_PRESS) {
        // Back to home
        request_close();
        return true;
    }
    return false;
}

} // namespace apps
} // namespace touchdown

// Register the app
REGISTER_APP(touchdown::apps::SettingsApp, "settings")
