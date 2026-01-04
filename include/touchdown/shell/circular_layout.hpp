/**
 * @file circular_layout.hpp
 * @brief Circular layout utilities for round display
 */

#ifndef TOUCHDOWN_SHELL_CIRCULAR_LAYOUT_HPP
#define TOUCHDOWN_SHELL_CIRCULAR_LAYOUT_HPP

#include "touchdown/core/types.hpp"
#include "lvgl.h"
#include <vector>

namespace touchdown {
namespace shell {

/**
 * @brief Calculate position on a circular arc
 */
struct ArcPosition {
    int16_t x;
    int16_t y;
    float angle_deg;
};

class CircularLayout {
public:
    /**
     * @brief Calculate positions for items arranged in a circle
     * @param count Number of items
     * @param radius Distance from center
     * @param start_angle Starting angle in degrees (0 = top)
     * @return Vector of positions
     */
    static std::vector<ArcPosition> calculate_circular_positions(
        int count, 
        uint16_t radius = DisplayConfig::SAFE_RADIUS - 30,
        float start_angle = -90.0f
    );
    
    /**
     * @brief Apply circular mask to an object
     */
    static void apply_circular_mask(lv_obj_t* obj);
    
    /**
     * @brief Create a circular container
     */
    static lv_obj_t* create_circular_container(lv_obj_t* parent);
    
    /**
     * @brief Position object at angle on circle
     */
    static void position_on_circle(lv_obj_t* obj, float angle_deg, uint16_t radius);
    
    /**
     * @brief Check if coordinates are in safe circular area
     */
    static bool is_in_safe_zone(int16_t x, int16_t y);
};

} // namespace shell
} // namespace touchdown

#endif // TOUCHDOWN_SHELL_CIRCULAR_LAYOUT_HPP
