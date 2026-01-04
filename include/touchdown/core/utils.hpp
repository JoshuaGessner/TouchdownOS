/**
 * @file utils.hpp
 * @brief Utility functions for TouchdownOS
 */

#ifndef TOUCHDOWN_CORE_UTILS_HPP
#define TOUCHDOWN_CORE_UTILS_HPP

#include "types.hpp"
#include <chrono>

namespace touchdown {

class Utils {
public:
    /**
     * @brief Check if a point is within a circular region
     */
    static bool is_point_in_circle(int16_t x, int16_t y, int16_t center_x, int16_t center_y, uint16_t radius);
    
    /**
     * @brief Get current timestamp in milliseconds
     */
    static uint32_t get_timestamp_ms();
    
    /**
     * @brief Calculate distance between two points
     */
    static float distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    
    /**
     * @brief Clamp value between min and max
     */
    template<typename T>
    static T clamp(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    /**
     * @brief Linear interpolation
     */
    template<typename T>
    static T lerp(T a, T b, float t) {
        return a + (b - a) * t;
    }
};

} // namespace touchdown

#endif // TOUCHDOWN_CORE_UTILS_HPP
