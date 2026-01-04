/**
 * @file input_service.hpp
 * @brief Input management service aggregating touch and button input
 */

#ifndef TOUCHDOWN_SERVICES_INPUT_SERVICE_HPP
#define TOUCHDOWN_SERVICES_INPUT_SERVICE_HPP

#include "touchdown/services/dbus_interface.hpp"
#include "touchdown/core/types.hpp"
#include <memory>
#include <atomic>

namespace touchdown {

namespace drivers {
    class TouchDriver;
    class ButtonDriver;
}

namespace services {

class InputService : public DBusInterface {
public:
    InputService();
    ~InputService();
    
    /**
     * @brief Initialize input service
     */
    bool init(drivers::TouchDriver* touch, drivers::ButtonDriver* button);
    
    /**
     * @brief Main service loop
     */
    void run();
    
    /**
     * @brief Stop service
     */
    void stop();
    
private:
    void on_touch_event(const TouchPoint& point);
    void on_button_event(const ButtonEvent& event);
    
    // D-Bus method handlers
    DBusMessage* handle_get_last_touch(DBusMessage* msg);
    DBusMessage* handle_get_last_button(DBusMessage* msg);
    
    drivers::TouchDriver* touch_;
    drivers::ButtonDriver* button_;
    std::atomic<bool> running_;
    
    TouchPoint last_touch_;
    ButtonEvent last_button_;
};

} // namespace services
} // namespace touchdown

#endif // TOUCHDOWN_SERVICES_INPUT_SERVICE_HPP
