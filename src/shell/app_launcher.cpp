/**
 * @file app_launcher.cpp
 * @brief App launcher implementation
 */

#include <algorithm>

#include "touchdown/shell/app_launcher.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/shell/circular_layout.hpp"
#include "touchdown/core/logger.hpp"

namespace touchdown {
namespace shell {

constexpr int APP_BUTTON_SIZE = 50;
constexpr int APP_ICON_SIZE = 32;

AppLauncher::AppLauncher()
    : container_(nullptr) {
}

AppLauncher::~AppLauncher() {
    if (container_) {
        lv_obj_del(container_);
    }
}

void AppLauncher::create(lv_obj_t* parent) {
    container_ = CircularLayout::create_circular_container(parent);
    
    auto& theme = ThemeEngine::instance();
    lv_obj_set_style_bg_color(container_, theme.get_palette().background, 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_90, 0);
    
    // Initially hidden
    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    
    TD_LOG_INFO("AppLauncher", "App launcher created");
}

void AppLauncher::add_app(const AppInfo& app) {
    apps_.push_back(app);
    refresh_layout();
    
    TD_LOG_INFO("AppLauncher", "Added app: ", app.name);
}

void AppLauncher::remove_app(const std::string& app_id) {
    auto it = std::remove_if(apps_.begin(), apps_.end(),
        [&app_id](const AppInfo& app) { return app.id == app_id; });
    
    if (it != apps_.end()) {
        apps_.erase(it, apps_.end());
        refresh_layout();
        TD_LOG_INFO("AppLauncher", "Removed app: ", app_id);
    }
}

void AppLauncher::refresh_layout() {
    // Clear existing buttons
    for (auto* btn : app_buttons_) {
        lv_obj_del(btn);
    }
    app_buttons_.clear();
    
    if (apps_.empty()) return;
    
    // Calculate positions in a circle
    auto positions = CircularLayout::calculate_circular_positions(
        apps_.size(), 
        DisplayConfig::SAFE_RADIUS - 40
    );
    
    // Create button for each app
    for (size_t i = 0; i < apps_.size(); i++) {
        create_app_button(apps_[i], positions[i].x, positions[i].y);
    }
}

void AppLauncher::create_app_button(const AppInfo& app, int16_t x, int16_t y) {
    auto& theme = ThemeEngine::instance();
    
    // Create button
    lv_obj_t* btn = lv_btn_create(container_);
    lv_obj_set_size(btn, APP_BUTTON_SIZE, APP_BUTTON_SIZE);
    lv_obj_set_pos(btn, x - APP_BUTTON_SIZE / 2, y - APP_BUTTON_SIZE / 2);
    
    lv_obj_set_style_radius(btn, APP_BUTTON_SIZE / 2, 0);  // Circular
    lv_obj_set_style_bg_color(btn, app.color, 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_30, 0);
    
    // Add icon/label
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, app.icon.c_str());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);
    
    // Store app ID in user data
    lv_obj_set_user_data(btn, (void*)app.id.c_str());
    
    // Add click event
    lv_obj_add_event_cb(btn, on_app_clicked, LV_EVENT_CLICKED, this);
    
    app_buttons_.push_back(btn);
}

void AppLauncher::on_app_clicked(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target_obj(e);
    AppLauncher* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    
    const char* app_id = static_cast<const char*>(lv_obj_get_user_data(btn));
    
    TD_LOG_INFO("AppLauncher", "App clicked: ", app_id);
    
    if (launcher && launcher->launch_callback_ && app_id) {
        launcher->launch_callback_(app_id);
    }
}

void AppLauncher::set_launch_callback(std::function<void(const std::string&)> callback) {
    launch_callback_ = callback;
}

void AppLauncher::show() {
    if (container_) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void AppLauncher::hide() {
    if (container_) {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void AppLauncher::animate_show(uint32_t duration_ms) {
    if (!container_) return;
    
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    
    // Fade in animation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, container_);
    lv_anim_set_values(&anim, 0, LV_OPA_COVER);
    lv_anim_set_time(&anim, duration_ms);
    lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
        lv_obj_set_style_opa((lv_obj_t*)var, value, 0);
    });
    lv_anim_start(&anim);
    
    // Scale in buttons
    for (auto* btn : app_buttons_) {
        lv_anim_t btn_anim;
        lv_anim_init(&btn_anim);
        lv_anim_set_var(&btn_anim, btn);
        lv_anim_set_values(&btn_anim, 0, 256);
        lv_anim_set_time(&btn_anim, duration_ms);
        lv_anim_set_exec_cb(&btn_anim, [](void* var, int32_t value) {
            lv_obj_set_style_transform_zoom((lv_obj_t*)var, value, 0);
        });
        lv_anim_start(&btn_anim);
    }
}

void AppLauncher::animate_hide(uint32_t duration_ms) {
    if (!container_) return;
    
    // Fade out animation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, container_);
    lv_anim_set_values(&anim, LV_OPA_COVER, 0);
    lv_anim_set_time(&anim, duration_ms);
    lv_anim_set_exec_cb(&anim, [](void* var, int32_t value) {
        lv_obj_set_style_opa((lv_obj_t*)var, value, 0);
    });
    lv_anim_set_ready_cb(&anim, [](lv_anim_t* a) {
        lv_obj_add_flag((lv_obj_t*)a->var, LV_OBJ_FLAG_HIDDEN);
    });
    lv_anim_start(&anim);
}

} // namespace shell
} // namespace touchdown
