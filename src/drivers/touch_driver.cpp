/**
 * @file touch_driver.cpp
 * @brief CST816S touch controller implementation
 */

#include "touchdown/drivers/touch_driver.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/utils.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstring>

namespace touchdown {
namespace drivers {

// CST816S register definitions
constexpr uint8_t REG_GESTURE_ID = 0x01;
constexpr uint8_t REG_TOUCH_NUM = 0x02;
constexpr uint8_t REG_XPOS_H = 0x03;
constexpr uint8_t REG_XPOS_L = 0x04;
constexpr uint8_t REG_YPOS_H = 0x05;
constexpr uint8_t REG_YPOS_L = 0x06;

constexpr uint32_t LONG_PRESS_THRESHOLD_MS = 500;
constexpr float SWIPE_THRESHOLD = 50.0f;

class TouchDriver::Impl {
public:
    int i2c_fd = -1;
    uint8_t address = 0x15;
    
    int16_t last_x = 0;
    int16_t last_y = 0;
    bool touched = false;
};

TouchDriver::TouchDriver() 
    : impl_(std::make_unique<Impl>())
    , indev_(nullptr)
    , touch_active_(false)
    , press_start_time_(0) {
}

TouchDriver::~TouchDriver() {
    deinit();
}

bool TouchDriver::init(const std::string& device, uint8_t address) {
    TD_LOG_INFO("TouchDriver", "Initializing touch controller: ", device);
    
    impl_->i2c_fd = open(device.c_str(), O_RDWR);
    if (impl_->i2c_fd < 0) {
        TD_LOG_ERROR("TouchDriver", "Failed to open I2C device: ", device);
        return false;
    }
    
    if (ioctl(impl_->i2c_fd, I2C_SLAVE, address) < 0) {
        TD_LOG_ERROR("TouchDriver", "Failed to set I2C slave address: ", (int)address);
        close(impl_->i2c_fd);
        return false;
    }
    
    impl_->address = address;
    
    // Initialize LVGL input device
    indev_ = lv_indev_create();
    if (!indev_) {
        TD_LOG_ERROR("TouchDriver", "Failed to create LVGL input device");
        close(impl_->i2c_fd);
        return false;
    }
    
    lv_indev_set_type(indev_, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_, read_cb);
    lv_indev_set_user_data(indev_, this);
    
    TD_LOG_INFO("TouchDriver", "Touch controller initialized");
    return true;
}

void TouchDriver::deinit() {
    if (impl_->i2c_fd >= 0) {
        close(impl_->i2c_fd);
        impl_->i2c_fd = -1;
    }
    
    TD_LOG_INFO("TouchDriver", "Touch controller deinitialized");
}

void TouchDriver::read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    TouchDriver* driver = static_cast<TouchDriver*>(lv_indev_get_user_data(indev));
    driver->read_touch(data);
}

void TouchDriver::read_touch(lv_indev_data_t* data) {
    if (impl_->i2c_fd < 0) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    // Read touch data from CST816S
    uint8_t buf[6];
    uint8_t reg = REG_GESTURE_ID;
    
    if (write(impl_->i2c_fd, &reg, 1) != 1 || read(impl_->i2c_fd, buf, 6) != 6) {
        data->state = LV_INDEV_STATE_RELEASED;
        impl_->touched = false;
        return;
    }
    
    uint8_t touch_num = buf[1];
    
    if (touch_num > 0) {
        // Extract coordinates
        int16_t x = ((buf[2] & 0x0F) << 8) | buf[3];
        int16_t y = ((buf[4] & 0x0F) << 8) | buf[5];
        
        // Apply coordinate transformations for circular display
        // (Inversion handled via device tree, but double-check here)
        x = DisplayConfig::WIDTH - x;
        y = DisplayConfig::HEIGHT - y;
        
        // Clamp to display bounds
        x = Utils::clamp<int16_t>(x, 0, DisplayConfig::WIDTH - 1);
        y = Utils::clamp<int16_t>(y, 0, DisplayConfig::HEIGHT - 1);
        
        impl_->last_x = x;
        impl_->last_y = y;
        impl_->touched = true;
        
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        
        // Gesture detection
        TouchPoint point = {x, y, TouchEventType::MOVE, Utils::get_timestamp_ms()};
        
        if (!touch_active_) {
            touch_active_ = true;
            press_start_time_ = point.timestamp_ms;
            last_press_ = point;
            point.type = TouchEventType::PRESS;
        }
        
        last_point_ = point;
        detect_gestures(point);
        
        if (touch_callback_) {
            touch_callback_(point);
        }
    } else {
        data->point.x = impl_->last_x;
        data->point.y = impl_->last_y;
        data->state = LV_INDEV_STATE_RELEASED;
        
        if (impl_->touched) {
            // Touch release
            impl_->touched = false;
            
            if (touch_active_) {
                TouchPoint point = {impl_->last_x, impl_->last_y, TouchEventType::RELEASE, Utils::get_timestamp_ms()};
                
                uint32_t duration = point.timestamp_ms - press_start_time_;
                if (duration >= LONG_PRESS_THRESHOLD_MS) {
                    point.type = TouchEventType::LONG_PRESS;
                } else {
                    point.type = TouchEventType::TAP;
                }
                
                if (touch_callback_) {
                    touch_callback_(point);
                }
                
                touch_active_ = false;
            }
        }
    }
}

void TouchDriver::detect_gestures(const TouchPoint& point) {
    if (point.type != TouchEventType::MOVE) return;
    
    float dx = point.x - last_press_.x;
    float dy = point.y - last_press_.y;
    float distance = Utils::distance(last_press_.x, last_press_.y, point.x, point.y);
    
    if (distance > SWIPE_THRESHOLD) {
        TouchEventType gesture = TouchEventType::MOVE;
        
        if (std::abs(dx) > std::abs(dy)) {
            gesture = (dx > 0) ? TouchEventType::SWIPE_RIGHT : TouchEventType::SWIPE_LEFT;
        } else {
            gesture = (dy > 0) ? TouchEventType::SWIPE_DOWN : TouchEventType::SWIPE_UP;
        }
        
        TouchPoint gesture_point = point;
        gesture_point.type = gesture;
        
        if (touch_callback_) {
            touch_callback_(gesture_point);
        }
        
        // Reset swipe detection
        last_press_ = point;
    }
}

void TouchDriver::set_touch_callback(TouchCallback callback) {
    touch_callback_ = callback;
}

void TouchDriver::set_sensitivity(uint8_t sensitivity) {
    // CST816S sensitivity adjustment (if supported by firmware)
    TD_LOG_DEBUG("TouchDriver", "Set sensitivity: ", (int)sensitivity);
}

} // namespace drivers
} // namespace touchdown
