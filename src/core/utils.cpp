/**
 * @file utils.cpp
 * @brief Utility functions
 */

#include "touchdown/core/utils.hpp"
#include <cmath>

namespace touchdown {

bool Utils::is_point_in_circle(int16_t x, int16_t y, int16_t center_x, int16_t center_y, uint16_t radius) {
    int32_t dx = x - center_x;
    int32_t dy = y - center_y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

uint32_t Utils::get_timestamp_ms() {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<uint32_t>(ms.count());
}

float Utils::distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    int32_t dx = x2 - x1;
    int32_t dy = y2 - y1;
    return std::sqrt(static_cast<float>(dx * dx + dy * dy));
}

} // namespace touchdown
