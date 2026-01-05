/**
 * @file circular_layout.cpp
 * @brief Circular layout implementation
 */

#include "touchdown/shell/circular_layout.hpp"
#include "touchdown/core/utils.hpp"
#include <cmath>

namespace touchdown {
namespace shell {

std::vector<ArcPosition> CircularLayout::calculate_circular_positions(
    int count, uint16_t radius, float start_angle) {
    
    std::vector<ArcPosition> positions;
    positions.reserve(count);
    
    float angle_step = 360.0f / count;
    
    for (int i = 0; i < count; i++) {
        float angle = start_angle + (i * angle_step);
        float angle_rad = angle * M_PI / 180.0f;
        
        int16_t x = DisplayConfig::CENTER_X + static_cast<int16_t>(radius * cos(angle_rad));
        int16_t y = DisplayConfig::CENTER_Y + static_cast<int16_t>(radius * sin(angle_rad));
        
        positions.push_back({x, y, angle});
    }
    
    return positions;
}

void CircularLayout::apply_circular_mask(lv_obj_t* obj) {
    if (!obj) return;

    // Round the container to the display radius and clip children to the circle
    lv_obj_set_style_radius(obj, DisplayConfig::RADIUS, 0);
    lv_obj_set_style_clip_corner(obj, true, 0);
}

lv_obj_t* CircularLayout::create_circular_container(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    
    lv_obj_set_size(cont, DisplayConfig::WIDTH, DisplayConfig::HEIGHT);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_style_radius(cont, DisplayConfig::RADIUS, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    
    return cont;
}

void CircularLayout::position_on_circle(lv_obj_t* obj, float angle_deg, uint16_t radius) {
    float angle_rad = angle_deg * M_PI / 180.0f;
    
    int16_t x = DisplayConfig::CENTER_X + static_cast<int16_t>(radius * cos(angle_rad));
    int16_t y = DisplayConfig::CENTER_Y + static_cast<int16_t>(radius * sin(angle_rad));
    
    // Center the object on the calculated position
    lv_obj_align(obj, LV_ALIGN_CENTER, 
                 x - DisplayConfig::CENTER_X, 
                 y - DisplayConfig::CENTER_Y);
}

bool CircularLayout::is_in_safe_zone(int16_t x, int16_t y) {
    return Utils::is_point_in_circle(x, y, 
                                    DisplayConfig::CENTER_X, 
                                    DisplayConfig::CENTER_Y,
                                    DisplayConfig::SAFE_RADIUS);
}

} // namespace shell
} // namespace touchdown
