/**
 * @file types.hpp
 * @brief Core type definitions for TouchdownOS
 */

#ifndef TOUCHDOWN_CORE_TYPES_HPP
#define TOUCHDOWN_CORE_TYPES_HPP

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace touchdown {

/**
 * @brief Display dimensions for the round screen
 */
struct DisplayConfig {
    static constexpr uint16_t WIDTH = 240;
    static constexpr uint16_t HEIGHT = 240;
    static constexpr uint16_t CENTER_X = WIDTH / 2;
    static constexpr uint16_t CENTER_Y = HEIGHT / 2;
    static constexpr uint16_t RADIUS = WIDTH / 2;
    static constexpr uint16_t SAFE_RADIUS = RADIUS - 10;  // 10px margin from edge
};

/**
 * @brief Touch input event types
 */
enum class TouchEventType {
    PRESS,
    RELEASE,
    MOVE,
    TAP,
    LONG_PRESS,
    SWIPE_UP,
    SWIPE_DOWN,
    SWIPE_LEFT,
    SWIPE_RIGHT
};

/**
 * @brief Touch point coordinates
 */
struct TouchPoint {
    int16_t x;
    int16_t y;
    TouchEventType type;
    uint32_t timestamp_ms;
};

/**
 * @brief Button event types
 */
enum class ButtonEventType {
    SINGLE_PRESS,
    DOUBLE_PRESS,
    LONG_PRESS,
    RELEASE
};

/**
 * @brief Button input event
 */
struct ButtonEvent {
    ButtonEventType type;
    uint32_t timestamp_ms;
    uint16_t duration_ms;
};

/**
 * @brief Power management states
 */
enum class PowerState {
    ACTIVE,
    SCREEN_OFF,
    SUSPENDED,
    SHUTDOWN
};

/**
 * @brief Battery information
 */
struct BatteryInfo {
    uint8_t percentage;      // 0-100
    bool charging;
    int16_t voltage_mv;
    int16_t current_ma;
};

/**
 * @brief Network connection status
 */
enum class NetworkStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

/**
 * @brief Network information
 */
struct NetworkInfo {
    NetworkStatus wifi_status;
    NetworkStatus bluetooth_status;
    std::string wifi_ssid;
    int8_t wifi_signal_strength;  // dBm
};

/**
 * @brief Result type for error handling
 */
template<typename T>
class Result {
public:
    static Result Ok(T value) {
        return Result(std::move(value), true, "");
    }
    
    static Result Error(const std::string& error) {
        return Result(T{}, false, error);
    }
    
    bool is_ok() const { return ok_; }
    bool is_error() const { return !ok_; }
    
    const T& value() const { return value_; }
    T& value() { return value_; }
    
    const std::string& error() const { return error_; }
    
private:
    Result(T value, bool ok, const std::string& error)
        : value_(std::move(value)), ok_(ok), error_(error) {}
    
    T value_;
    bool ok_;
    std::string error_;
};

/**
 * @brief Callback types
 */
using TouchCallback = std::function<void(const TouchPoint&)>;
using ButtonCallback = std::function<void(const ButtonEvent&)>;
using PowerStateCallback = std::function<void(PowerState)>;

} // namespace touchdown

#endif // TOUCHDOWN_CORE_TYPES_HPP
