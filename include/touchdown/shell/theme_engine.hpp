/**
 * @file theme_engine.hpp
 * @brief Theme management for TouchdownOS
 */

#ifndef TOUCHDOWN_SHELL_THEME_ENGINE_HPP
#define TOUCHDOWN_SHELL_THEME_ENGINE_HPP

#include "lvgl.h"
#include <string>
#include <map>

namespace touchdown {
namespace shell {

struct ColorPalette {
    lv_color_t primary;
    lv_color_t secondary;
    lv_color_t accent;
    lv_color_t background;
    lv_color_t surface;
    lv_color_t text_primary;
    lv_color_t text_secondary;
    lv_color_t success;
    lv_color_t warning;
    lv_color_t error;
};

enum class ThemeMode {
    LIGHT,
    DARK,
    AUTO
};

class ThemeEngine {
public:
    static ThemeEngine& instance();
    
    /**
     * @brief Initialize theme engine
     */
    void init();
    
    /**
     * @brief Set theme mode
     */
    void set_mode(ThemeMode mode);
    
    /**
     * @brief Get current theme mode
     */
    ThemeMode get_mode() const { return current_mode_; }
    
    /**
     * @brief Get current color palette
     */
    const ColorPalette& get_palette() const { return current_palette_; }
    
    /**
     * @brief Apply theme to LVGL
     */
    void apply_theme();
    
    /**
     * @brief Create themed style for a component
     */
    lv_style_t create_card_style();
    lv_style_t create_button_style();
    lv_style_t create_text_style(bool secondary = false);
    
    /**
     * @brief Smooth transition between themes
     */
    void animate_theme_change(ThemeMode new_mode, uint32_t duration_ms = 300);
    
private:
    ThemeEngine() = default;
    
    void load_dark_palette();
    void load_light_palette();
    
    ThemeMode current_mode_;
    ColorPalette current_palette_;
    lv_theme_t* lvgl_theme_;
};

} // namespace shell
} // namespace touchdown

#endif // TOUCHDOWN_SHELL_THEME_ENGINE_HPP
