/**
 * @file touch_driver.hpp
 * @brief CST816S I2C touch controller driver
 */

#ifndef TOUCHDOWN_DRIVERS_TOUCH_DRIVER_HPP
#define TOUCHDOWN_DRIVERS_TOUCH_DRIVER_HPP

#include "touchdown/core/types.hpp"
#include "lvgl.h"
#include <memory>
#include <functional>

namespace touchdown {
namespace drivers {

class TouchDriver {
public:
    TouchDriver();
    ~TouchDriver();
    
    /**
     * @brief Initialize touch controller
     * @param device I2C device path (e.g., "/dev/i2c-1")
     * @param address I2C address (default: 0x15)
     * @return true on success
     */
    bool init(const std::string& device = "/dev/i2c-1", uint8_t address = 0x15);
    
    /**
     * @brief Clean up touch resources
     */
    void deinit();
    
    /**
     * @brief Get LVGL input device
     */
    lv_indev_t* get_input_device() { return indev_; }
    
    /**
     * @brief Register callback for touch events
     */
    void set_touch_callback(TouchCallback callback);
    
    /**
     * @brief Set touch sensitivity (0-255)
     */
    void set_sensitivity(uint8_t sensitivity);
    
private:
    static void read_cb(lv_indev_t* indev, lv_indev_data_t* data);
    void read_touch(lv_indev_data_t* data);
    
    // Gesture detection
    void detect_gestures(const TouchPoint& point);
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    lv_indev_t* indev_;
    TouchCallback touch_callback_;
    
    // Gesture state
    TouchPoint last_press_;
    TouchPoint last_point_;
    bool touch_active_;
    uint32_t press_start_time_;
};

} // namespace drivers
} // namespace touchdown

#endif // TOUCHDOWN_DRIVERS_TOUCH_DRIVER_HPP
