/**
 * @file display_driver.hpp
 * @brief DRM/KMS display driver interface for LVGL
 */

#ifndef TOUCHDOWN_DRIVERS_DISPLAY_DRIVER_HPP
#define TOUCHDOWN_DRIVERS_DISPLAY_DRIVER_HPP

#include "touchdown/core/types.hpp"
#include "lvgl.h"
#include <memory>
#include <string>

namespace touchdown {
namespace drivers {

class DisplayDriver {
public:
    DisplayDriver();
    ~DisplayDriver();
    
    /**
     * @brief Initialize DRM display
     * @param device DRM device path (e.g., "/dev/dri/card0")
     * @return true on success
     */
    bool init(const std::string& device = "/dev/dri/card0");
    
    /**
     * @brief Clean up display resources
     */
    void deinit();
    
    /**
     * @brief Get LVGL display object
     */
    lv_display_t* get_display() { return display_; }
    
    /**
     * @brief Set display brightness (0-255)
     */
    void set_brightness(uint8_t brightness);
    
    /**
     * @brief Turn display on/off
     */
    void set_power(bool on);
    
    /**
     * @brief Check if point is within safe display area (circular mask)
     */
    bool is_point_safe(int16_t x, int16_t y);
    
private:
    static void flush_cb(lv_display_t* disp, const lv_area_t* area, unsigned char* color_p);
    void flush_display(const lv_area_t* area, unsigned char* color_p);
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    lv_display_t* display_;
};

} // namespace drivers
} // namespace touchdown

#endif // TOUCHDOWN_DRIVERS_DISPLAY_DRIVER_HPP
