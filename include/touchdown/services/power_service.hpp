/**
 * @file power_service.hpp
 * @brief Power management service with D-Bus interface
 */

#ifndef TOUCHDOWN_SERVICES_POWER_SERVICE_HPP
#define TOUCHDOWN_SERVICES_POWER_SERVICE_HPP

#include "touchdown/services/dbus_interface.hpp"
#include "touchdown/core/types.hpp"
#include <memory>
#include <atomic>

namespace touchdown {

// Forward declarations
namespace drivers {
    class DisplayDriver;
}

namespace services {

class PowerService : public DBusInterface {
public:
    PowerService();
    ~PowerService();
    
    /**
     * @brief Initialize power service
     */
    bool init(drivers::DisplayDriver* display);
    
    /**
     * @brief Main service loop
     */
    void run();
    
    /**
     * @brief Stop service
     */
    void stop();
    
    /**
     * @brief Set power state
     */
    void set_power_state(PowerState state);
    
    /**
     * @brief Get current power state
     */
    PowerState get_power_state() const { return power_state_; }
    
    /**
     * @brief Set screen timeout (milliseconds, 0 = disabled)
     */
    void set_screen_timeout(uint32_t timeout_ms);
    
    /**
     * @brief Reset idle timer (called on user activity)
     */
    void reset_idle_timer();
    
private:
    void apply_power_state(PowerState state);
    void apply_cpu_scaling(const std::string& governor);
    void check_idle_timeout();
    
    // D-Bus method handlers
    DBusMessage* handle_set_power_state(DBusMessage* msg);
    DBusMessage* handle_get_power_state(DBusMessage* msg);
    DBusMessage* handle_set_screen_timeout(DBusMessage* msg);
    DBusMessage* handle_reset_idle_timer(DBusMessage* msg);
    
    drivers::DisplayDriver* display_;
    PowerState power_state_;
    std::atomic<bool> running_;
    
    uint32_t screen_timeout_ms_;
    uint32_t last_activity_time_;
};

} // namespace services
} // namespace touchdown

#endif // TOUCHDOWN_SERVICES_POWER_SERVICE_HPP
