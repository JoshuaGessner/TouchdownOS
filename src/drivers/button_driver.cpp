/**
 * @file button_driver.cpp
 * @brief GPIO button driver implementation
 */

#include "touchdown/drivers/button_driver.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/utils.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <thread>
#include <chrono>

namespace touchdown {
namespace drivers {

class ButtonDriver::Impl {
public:
    int event_fd = -1;
    int gpio_pin = 23;
};

ButtonDriver::ButtonDriver()
    : impl_(std::make_unique<Impl>())
    , running_(false)
    , last_state_(false)
    , press_start_time_(0)
    , last_press_time_(0)
    , waiting_for_double_(false)
    , debounce_ms_(50)
    , double_press_window_ms_(300)
    , long_press_threshold_ms_(500) {
}

ButtonDriver::~ButtonDriver() {
    deinit();
}

bool ButtonDriver::init(int gpio_pin) {
    LOG_INFO("ButtonDriver", "Initializing button on GPIO: ", gpio_pin);
    
    impl_->gpio_pin = gpio_pin;
    
    // Open event device for button (configured via device tree)
    // Look for the power button event
    for (int i = 0; i < 10; i++) {
        std::string device = "/dev/input/event" + std::to_string(i);
        int fd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
        
        if (fd >= 0) {
            char name[256] = {0};
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
                std::string device_name(name);
                if (device_name.find("Power Button") != std::string::npos ||
                    device_name.find("touchdown-button") != std::string::npos) {
                    impl_->event_fd = fd;
                    LOG_INFO("ButtonDriver", "Found button device: ", device_name);
                    break;
                }
            }
            close(fd);
        }
    }
    
    if (impl_->event_fd < 0) {
        LOG_ERROR("ButtonDriver", "Failed to find button event device");
        return false;
    }
    
    // Start monitoring thread
    running_ = true;
    monitor_thread_ = std::thread(&ButtonDriver::monitor_thread, this);
    
    LOG_INFO("ButtonDriver", "Button driver initialized");
    return true;
}

void ButtonDriver::deinit() {
    running_ = false;
    
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    
    if (impl_->event_fd >= 0) {
        close(impl_->event_fd);
        impl_->event_fd = -1;
    }
    
    LOG_INFO("ButtonDriver", "Button driver deinitialized");
}

void ButtonDriver::monitor_thread() {
    struct input_event ev;
    fd_set fds;
    struct timeval tv;
    
    while (running_) {
        FD_ZERO(&fds);
        FD_SET(impl_->event_fd, &fds);
        
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100ms timeout
        
        int ret = select(impl_->event_fd + 1, &fds, nullptr, nullptr, &tv);
        
        if (ret > 0 && FD_ISSET(impl_->event_fd, &fds)) {
            if (read(impl_->event_fd, &ev, sizeof(ev)) == sizeof(ev)) {
                if (ev.type == EV_KEY && ev.code == KEY_POWER) {
                    bool pressed = (ev.value == 1);
                    process_button_event(pressed);
                }
            }
        }
        
        // Check for double-press timeout
        if (waiting_for_double_) {
            uint32_t now = Utils::get_timestamp_ms();
            if (now - last_press_time_ > double_press_window_ms_) {
                // Timeout - emit single press
                ButtonEvent event = {ButtonEventType::SINGLE_PRESS, last_press_time_, 0};
                if (button_callback_) {
                    button_callback_(event);
                }
                waiting_for_double_ = false;
            }
        }
    }
}

void ButtonDriver::process_button_event(bool pressed) {
    uint32_t now = Utils::get_timestamp_ms();
    
    if (pressed && !last_state_) {
        // Button pressed
        press_start_time_ = now;
        last_state_ = true;
        
        LOG_DEBUG("ButtonDriver", "Button pressed");
        
    } else if (!pressed && last_state_) {
        // Button released
        uint32_t duration = now - press_start_time_;
        last_state_ = false;
        
        LOG_DEBUG("ButtonDriver", "Button released, duration: ", duration, "ms");
        
        if (duration >= long_press_threshold_ms_) {
            // Long press
            ButtonEvent event = {ButtonEventType::LONG_PRESS, now, static_cast<uint16_t>(duration)};
            if (button_callback_) {
                button_callback_(event);
            }
            waiting_for_double_ = false;
            
        } else {
            // Short press - check for double press
            if (waiting_for_double_ && (now - last_press_time_) < double_press_window_ms_) {
                // Double press detected
                ButtonEvent event = {ButtonEventType::DOUBLE_PRESS, now, 0};
                if (button_callback_) {
                    button_callback_(event);
                }
                waiting_for_double_ = false;
            } else {
                // Wait for potential double press
                last_press_time_ = now;
                waiting_for_double_ = true;
            }
        }
        
        // Release event
        ButtonEvent event = {ButtonEventType::RELEASE, now, static_cast<uint16_t>(duration)};
        if (button_callback_) {
            button_callback_(event);
        }
    }
}

void ButtonDriver::set_button_callback(ButtonCallback callback) {
    button_callback_ = callback;
}

void ButtonDriver::set_double_press_window_ms(uint32_t ms) {
    double_press_window_ms_ = ms;
}

void ButtonDriver::set_long_press_threshold_ms(uint32_t ms) {
    long_press_threshold_ms_ = ms;
}

} // namespace drivers
} // namespace touchdown
