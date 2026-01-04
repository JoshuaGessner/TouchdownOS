/**
 * @file button_driver.hpp
 * @brief GPIO button driver with gesture detection
 */

#ifndef TOUCHDOWN_DRIVERS_BUTTON_DRIVER_HPP
#define TOUCHDOWN_DRIVERS_BUTTON_DRIVER_HPP

#include "touchdown/core/types.hpp"
#include <memory>
#include <thread>
#include <atomic>

namespace touchdown {
namespace drivers {

class ButtonDriver {
public:
    ButtonDriver();
    ~ButtonDriver();
    
    /**
     * @brief Initialize button driver
     * @param gpio_pin GPIO pin number
     * @return true on success
     */
    bool init(int gpio_pin = 23);
    
    /**
     * @brief Clean up button resources
     */
    void deinit();
    
    /**
     * @brief Register callback for button events
     */
    void set_button_callback(ButtonCallback callback);
    
    /**
     * @brief Configure button timing thresholds
     */
    void set_double_press_window_ms(uint32_t ms);
    void set_long_press_threshold_ms(uint32_t ms);
    
private:
    void monitor_thread();
    void process_button_event(bool pressed);
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    ButtonCallback button_callback_;
    
    std::thread monitor_thread_;
    std::atomic<bool> running_;
    
    // Button state
    bool last_state_;
    uint32_t press_start_time_;
    uint32_t last_press_time_;
    bool waiting_for_double_;
    
    // Configuration
    uint32_t debounce_ms_;
    uint32_t double_press_window_ms_;
    uint32_t long_press_threshold_ms_;
};

} // namespace drivers
} // namespace touchdown

#endif // TOUCHDOWN_DRIVERS_BUTTON_DRIVER_HPP
