/**
 * @file settings_app.cpp
 * @brief Settings app implementation
 */

#include "touchdown/apps/settings_app.hpp"
#include "touchdown/app/app_registry.hpp"
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
    TD_LOG_INFO("SettingsApp", "Initializing settings app");
    
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
    // Store callback and use index-based dispatch to avoid user-data type issues
    callbacks_.push_back(item.on_click);
    items_.push_back(btn);

    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
        if (!self) return;
        lv_obj_t* target = lv_event_get_target_obj(e);
        uint32_t idx = lv_obj_get_index(target);
        if (idx < self->callbacks_.size()) {
            self->callbacks_[idx]();
        }
    }, LV_EVENT_CLICKED, this);
}

void SettingsApp::on_theme_toggle() {
    TD_LOG_INFO("SettingsApp", "Toggle theme");
    
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
    TD_LOG_INFO("SettingsApp", "Adjust brightness");
    
    // TODO: Create brightness slider popup
    // For now, just toggle between 50% and 100%
    int current = Config::instance().get_int("display.brightness", 255);
    int new_brightness = (current == 255) ? 128 : 255;
    
    Config::instance().set_int("display.brightness", new_brightness);
    Config::instance().save("/etc/touchdown/shell.conf");
    
    // Send D-Bus message to power service to update brightness
}

void SettingsApp::on_about() {
    TD_LOG_INFO("SettingsApp", "Show about");
    
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
    TD_LOG_INFO("SettingsApp", "Cleanup");
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
namespace touchdown {
namespace apps {
REGISTER_APP(SettingsApp, "settings")
} // namespace apps
} // namespace touchdown
