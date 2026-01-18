/**
 * @file theme_engine.cpp
 * @brief Theme management implementation
 */

#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/core/logger.hpp"

namespace touchdown {
namespace shell {

ThemeEngine& ThemeEngine::instance() {
    static ThemeEngine engine;
    return engine;
}

void ThemeEngine::init() {
    current_mode_ = ThemeMode::DARK;
    load_dark_palette();
    apply_theme();
    
    TD_LOG_INFO("ThemeEngine", "Theme engine initialized");
}

void ThemeEngine::load_dark_palette() {
    current_palette_ = {
        .primary = lv_color_hex(0x00A8E8),      // Bright blue
        .secondary = lv_color_hex(0x007EA7),    // Deep blue
        .accent = lv_color_hex(0x00FFA3),       // Bright cyan
        .background = lv_color_hex(0x0A0A0A),   // Near black
        .surface = lv_color_hex(0x1A1A1A),      // Dark gray
        .text_primary = lv_color_hex(0xFFFFFF), // White
        .text_secondary = lv_color_hex(0xB0B0B0), // Light gray
        .success = lv_color_hex(0x00FF88),      // Green
        .warning = lv_color_hex(0xFFAA00),      // Orange
        .error = lv_color_hex(0xFF4444),        // Red
    };
}

void ThemeEngine::load_light_palette() {
    current_palette_ = {
        .primary = lv_color_hex(0x0088CC),      // Blue
        .secondary = lv_color_hex(0x005A8C),    // Dark blue
        .accent = lv_color_hex(0x00CC88),       // Teal
        .background = lv_color_hex(0xF5F5F5),   // Light gray
        .surface = lv_color_hex(0xFFFFFF),      // White
        .text_primary = lv_color_hex(0x212121), // Dark gray
        .text_secondary = lv_color_hex(0x757575), // Medium gray
        .success = lv_color_hex(0x4CAF50),      // Green
        .warning = lv_color_hex(0xFF9800),      // Orange
        .error = lv_color_hex(0xF44336),        // Red
    };
}

void ThemeEngine::set_mode(ThemeMode mode) {
    if (mode == current_mode_) return;
    
    TD_LOG_INFO("ThemeEngine", "Changing theme mode: ", static_cast<int>(mode));
    
    current_mode_ = mode;
    
    switch (mode) {
        case ThemeMode::DARK:
            load_dark_palette();
            break;
        case ThemeMode::LIGHT:
            load_light_palette();
            break;
        case ThemeMode::AUTO:
            // TODO: Determine based on time of day
            load_dark_palette();
            break;
    }
    
    apply_theme();
}

void ThemeEngine::apply_theme() {
    // Initialize LVGL default theme with our colors
    lvgl_theme_ = lv_theme_default_init(
        lv_display_get_default(),
        current_palette_.primary,
        current_palette_.secondary,
        current_mode_ == ThemeMode::DARK,
        LV_FONT_DEFAULT
    );
    
    lv_display_set_theme(lv_display_get_default(), lvgl_theme_);
    
    TD_LOG_INFO("ThemeEngine", "Theme applied");
}

lv_style_t ThemeEngine::create_card_style() {
    lv_style_t style;
    lv_style_init(&style);
    
    lv_style_set_bg_color(&style, current_palette_.surface);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_border_width(&style, 0);
    lv_style_set_radius(&style, 12);
    lv_style_set_pad_all(&style, 16);
    lv_style_set_shadow_width(&style, 8);
    lv_style_set_shadow_color(&style, lv_color_black());
    lv_style_set_shadow_opa(&style, LV_OPA_20);
    
    return style;
}

lv_style_t ThemeEngine::create_button_style() {
    lv_style_t style;
    lv_style_init(&style);
    
    lv_style_set_bg_color(&style, current_palette_.primary);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);
    lv_style_set_border_width(&style, 0);
    lv_style_set_radius(&style, 20);
    lv_style_set_pad_all(&style, 12);
    lv_style_set_text_color(&style, lv_color_white());
    
    return style;
}

lv_style_t ThemeEngine::create_text_style(bool secondary) {
    lv_style_t style;
    lv_style_init(&style);
    
    lv_style_set_text_color(&style, secondary ? 
        current_palette_.text_secondary : current_palette_.text_primary);
    lv_style_set_text_font(&style, LV_FONT_DEFAULT);
    
    return style;
}

void ThemeEngine::animate_theme_change(ThemeMode new_mode, uint32_t duration_ms) {
    // TODO: Implement smooth theme transition animation
    set_mode(new_mode);
}

} // namespace shell
} // namespace touchdown
