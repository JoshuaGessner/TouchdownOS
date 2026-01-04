/**
 * @file power_service.cpp
 * @brief Power management service implementation
 */

#include "touchdown/services/power_service.hpp"
#include "touchdown/drivers/display_driver.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/utils.hpp"
#include <fstream>
#include <thread>
#include <chrono>

namespace touchdown {
namespace services {

constexpr const char* DBUS_INTERFACE = "org.touchdown.Power";
constexpr const char* DBUS_OBJECT_PATH = "/org/touchdown/Power";

constexpr uint32_t DEFAULT_SCREEN_TIMEOUT_MS = 30000;  // 30 seconds

PowerService::PowerService()
    : DBusInterface("org.touchdown.Power", DBUS_OBJECT_PATH)
    , display_(nullptr)
    , power_state_(PowerState::ACTIVE)
    , running_(false)
    , screen_timeout_ms_(DEFAULT_SCREEN_TIMEOUT_MS)
    , last_activity_time_(0) {
}

PowerService::~PowerService() {
    stop();
}

bool PowerService::init(drivers::DisplayDriver* display) {
    display_ = display;
    
    if (!DBusInterface::init()) {
        return false;
    }
    
    // Register D-Bus methods
    register_method(DBUS_INTERFACE, "SetPowerState",
        [this](DBusMessage* msg) { return handle_set_power_state(msg); });
    
    register_method(DBUS_INTERFACE, "GetPowerState",
        [this](DBusMessage* msg) { return handle_get_power_state(msg); });
    
    register_method(DBUS_INTERFACE, "SetScreenTimeout",
        [this](DBusMessage* msg) { return handle_set_screen_timeout(msg); });
    
    register_method(DBUS_INTERFACE, "ResetIdleTimer",
        [this](DBusMessage* msg) { return handle_reset_idle_timer(msg); });
    
    // Set initial CPU governor
    apply_cpu_scaling("schedutil");
    
    last_activity_time_ = Utils::get_timestamp_ms();
    
    LOG_INFO("PowerService", "Power service initialized");
    return true;
}

void PowerService::run() {
    running_ = true;
    notify_ready();
    
    uint32_t watchdog_count = 0;
    
    while (running_) {
        // Process D-Bus messages
        process();
        
        // Check idle timeout
        check_idle_timeout();
        
        // Send watchdog keepalive every 10 seconds
        if (++watchdog_count >= 100) {  // 100 * 100ms = 10s
            send_watchdog();
            watchdog_count = 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void PowerService::stop() {
    running_ = false;
}

void PowerService::set_power_state(PowerState state) {
    if (power_state_ == state) return;
    
    LOG_INFO("PowerService", "Changing power state: ", static_cast<int>(power_state_), 
             " -> ", static_cast<int>(state));
    
    power_state_ = state;
    apply_power_state(state);
    
    // Notify via D-Bus signal
    std::string state_str;
    switch (state) {
        case PowerState::ACTIVE: state_str = "active"; break;
        case PowerState::SCREEN_OFF: state_str = "screen_off"; break;
        case PowerState::SUSPENDED: state_str = "suspended"; break;
        case PowerState::SHUTDOWN: state_str = "shutdown"; break;
    }
    send_signal(DBUS_INTERFACE, "PowerStateChanged", state_str);
}

void PowerService::apply_power_state(PowerState state) {
    switch (state) {
        case PowerState::ACTIVE:
            if (display_) {
                display_->set_power(true);
                display_->set_brightness(255);
            }
            apply_cpu_scaling("schedutil");
            break;
            
        case PowerState::SCREEN_OFF:
            if (display_) {
                display_->set_power(false);
            }
            apply_cpu_scaling("powersave");
            break;
            
        case PowerState::SUSPENDED:
            // Full system suspend (not implemented yet)
            LOG_WARNING("PowerService", "System suspend not yet implemented");
            break;
            
        case PowerState::SHUTDOWN:
            LOG_INFO("PowerService", "Initiating system shutdown");
            // Trigger systemd shutdown
            system("systemctl poweroff");
            break;
    }
}

void PowerService::apply_cpu_scaling(const std::string& governor) {
    // Apply CPU frequency governor to all cores
    for (int cpu = 0; cpu < 4; cpu++) {  // Pi Zero 2 W has 4 cores
        std::string path = "/sys/devices/system/cpu/cpu" + std::to_string(cpu) + 
                          "/cpufreq/scaling_governor";
        std::ofstream file(path);
        if (file.is_open()) {
            file << governor;
            file.close();
            LOG_DEBUG("PowerService", "Set CPU", cpu, " governor: ", governor);
        }
    }
}

void PowerService::check_idle_timeout() {
    if (screen_timeout_ms_ == 0) return;  // Timeout disabled
    if (power_state_ != PowerState::ACTIVE) return;  // Already in power saving
    
    uint32_t now = Utils::get_timestamp_ms();
    uint32_t idle_time = now - last_activity_time_;
    
    if (idle_time >= screen_timeout_ms_) {
        LOG_INFO("PowerService", "Screen timeout reached, turning off display");
        set_power_state(PowerState::SCREEN_OFF);
    }
}

void PowerService::set_screen_timeout(uint32_t timeout_ms) {
    screen_timeout_ms_ = timeout_ms;
    LOG_INFO("PowerService", "Screen timeout set to: ", timeout_ms, "ms");
}

void PowerService::reset_idle_timer() {
    last_activity_time_ = Utils::get_timestamp_ms();
    
    // Wake screen if it was off
    if (power_state_ == PowerState::SCREEN_OFF) {
        set_power_state(PowerState::ACTIVE);
    }
}

DBusMessage* PowerService::handle_set_power_state(DBusMessage* msg) {
    const char* state_str = nullptr;
    dbus_message_get_args(msg, nullptr, DBUS_TYPE_STRING, &state_str, DBUS_TYPE_INVALID);
    
    if (state_str) {
        PowerState state;
        if (strcmp(state_str, "active") == 0) state = PowerState::ACTIVE;
        else if (strcmp(state_str, "screen_off") == 0) state = PowerState::SCREEN_OFF;
        else if (strcmp(state_str, "suspended") == 0) state = PowerState::SUSPENDED;
        else if (strcmp(state_str, "shutdown") == 0) state = PowerState::SHUTDOWN;
        else return dbus_message_new_error(msg, "org.touchdown.Error", "Invalid state");
        
        set_power_state(state);
    }
    
    return dbus_message_new_method_return(msg);
}

DBusMessage* PowerService::handle_get_power_state(DBusMessage* msg) {
    DBusMessage* reply = dbus_message_new_method_return(msg);
    
    const char* state_str;
    switch (power_state_) {
        case PowerState::ACTIVE: state_str = "active"; break;
        case PowerState::SCREEN_OFF: state_str = "screen_off"; break;
        case PowerState::SUSPENDED: state_str = "suspended"; break;
        case PowerState::SHUTDOWN: state_str = "shutdown"; break;
    }
    
    dbus_message_append_args(reply, DBUS_TYPE_STRING, &state_str, DBUS_TYPE_INVALID);
    return reply;
}

DBusMessage* PowerService::handle_set_screen_timeout(DBusMessage* msg) {
    uint32_t timeout_ms = 0;
    dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &timeout_ms, DBUS_TYPE_INVALID);
    
    set_screen_timeout(timeout_ms);
    return dbus_message_new_method_return(msg);
}

DBusMessage* PowerService::handle_reset_idle_timer(DBusMessage* msg) {
    reset_idle_timer();
    return dbus_message_new_method_return(msg);
}

} // namespace services
} // namespace touchdown
