/**
 * @file input_service.cpp
 * @brief Input management service implementation
 */

#include "touchdown/services/input_service.hpp"
#include "touchdown/drivers/touch_driver.hpp"
#include "touchdown/drivers/button_driver.hpp"
#include "touchdown/core/logger.hpp"
#include <thread>
#include <chrono>
#include <cstring>

namespace touchdown {
namespace services {

constexpr const char* DBUS_INTERFACE = "org.touchdown.Input";
constexpr const char* DBUS_OBJECT_PATH = "/org/touchdown/Input";

InputService::InputService()
    : DBusInterface("org.touchdown.Input", DBUS_OBJECT_PATH)
    , touch_(nullptr)
    , button_(nullptr)
    , running_(false)
    , last_touch_{}
    , last_button_{} {
}

InputService::~InputService() {
    stop();
}

bool InputService::init(drivers::TouchDriver* touch, drivers::ButtonDriver* button) {
    touch_ = touch;
    button_ = button;
    
    if (!DBusInterface::init()) {
        return false;
    }
    
    // Register D-Bus methods
    register_method(DBUS_INTERFACE, "GetLastTouch",
        [this](DBusMessage* msg) { return handle_get_last_touch(msg); });
    
    register_method(DBUS_INTERFACE, "GetLastButton",
        [this](DBusMessage* msg) { return handle_get_last_button(msg); });
    
    // Register input callbacks
    if (touch_) {
        touch_->set_touch_callback([this](const TouchPoint& p) { on_touch_event(p); });
    }
    
    if (button_) {
        button_->set_button_callback([this](const ButtonEvent& e) { on_button_event(e); });
    }
    
    TD_LOG_INFO("InputService", "Input service initialized");
    return true;
}

void InputService::run() {
    running_ = true;
    notify_ready();
    
    uint32_t watchdog_count = 0;
    
    while (running_) {
        // Process D-Bus messages
        process();
        
        // Send watchdog keepalive every 10 seconds
        if (++watchdog_count >= 100) {
            send_watchdog();
            watchdog_count = 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void InputService::stop() {
    running_ = false;
}

void InputService::on_touch_event(const TouchPoint& point) {
    last_touch_ = point;
    
    // Send D-Bus signal for touch events
    std::string event_type;
    switch (point.type) {
        case TouchEventType::PRESS: event_type = "press"; break;
        case TouchEventType::RELEASE: event_type = "release"; break;
        case TouchEventType::MOVE: event_type = "move"; break;
        case TouchEventType::TAP: event_type = "tap"; break;
        case TouchEventType::LONG_PRESS: event_type = "long_press"; break;
        case TouchEventType::SWIPE_UP: event_type = "swipe_up"; break;
        case TouchEventType::SWIPE_DOWN: event_type = "swipe_down"; break;
        case TouchEventType::SWIPE_LEFT: event_type = "swipe_left"; break;
        case TouchEventType::SWIPE_RIGHT: event_type = "swipe_right"; break;
    }
    
    char signal_data[128];
    snprintf(signal_data, sizeof(signal_data), "%s,%d,%d,%u",
             event_type.c_str(), point.x, point.y, point.timestamp_ms);
    
    send_signal(DBUS_INTERFACE, "TouchEvent", signal_data);
    
    TD_LOG_DEBUG("InputService", "Touch event: ", event_type, " at (", point.x, ",", point.y, ")");
}

void InputService::on_button_event(const ButtonEvent& event) {
    last_button_ = event;
    
    // Send D-Bus signal for button events
    std::string event_type;
    switch (event.type) {
        case ButtonEventType::SINGLE_PRESS: event_type = "single_press"; break;
        case ButtonEventType::DOUBLE_PRESS: event_type = "double_press"; break;
        case ButtonEventType::LONG_PRESS: event_type = "long_press"; break;
        case ButtonEventType::RELEASE: event_type = "release"; break;
    }
    
    char signal_data[128];
    snprintf(signal_data, sizeof(signal_data), "%s,%u,%u",
             event_type.c_str(), event.timestamp_ms, event.duration_ms);
    
    send_signal(DBUS_INTERFACE, "ButtonEvent", signal_data);
    
    TD_LOG_INFO("InputService", "Button event: ", event_type);
}

DBusMessage* InputService::handle_get_last_touch(DBusMessage* msg) {
    DBusMessage* reply = dbus_message_new_method_return(msg);
    
    dbus_message_append_args(reply,
        DBUS_TYPE_INT16, &last_touch_.x,
        DBUS_TYPE_INT16, &last_touch_.y,
        DBUS_TYPE_UINT32, &last_touch_.timestamp_ms,
        DBUS_TYPE_INVALID);
    
    return reply;
}

DBusMessage* InputService::handle_get_last_button(DBusMessage* msg) {
    DBusMessage* reply = dbus_message_new_method_return(msg);
    
    uint32_t type = static_cast<uint32_t>(last_button_.type);
    dbus_message_append_args(reply,
        DBUS_TYPE_UINT32, &type,
        DBUS_TYPE_UINT32, &last_button_.timestamp_ms,
        DBUS_TYPE_UINT16, &last_button_.duration_ms,
        DBUS_TYPE_INVALID);
    
    return reply;
}

} // namespace services
} // namespace touchdown
