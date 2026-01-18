/**
 * @file app.cpp
 * @brief TouchdownApp base class implementation
 */

#include "touchdown/app/app.hpp"
#include "touchdown/shell/circular_layout.hpp"
#include "touchdown/core/logger.hpp"
#include <algorithm>

namespace touchdown {
namespace app {

TouchdownApp::TouchdownApp(const AppMetadata& metadata)
    : metadata_(metadata)
    , container_(nullptr)
    , visible_(false)
    , paused_(false) {
}

TouchdownApp::~TouchdownApp() {
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

lv_obj_t* TouchdownApp::create_container(lv_obj_t* parent) {
    container_ = shell::CircularLayout::create_circular_container(parent);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);  // Start hidden
    return container_;
}

bool TouchdownApp::has_permission(const std::string& permission) const {
    auto it = std::find(metadata_.permissions.begin(), 
                       metadata_.permissions.end(), 
                       permission);
    return it != metadata_.permissions.end();
}

void TouchdownApp::request_close() {
    // Signal shell to close this app
    TD_LOG_INFO("TouchdownApp", "App requests close: ", metadata_.id);
    // This would send D-Bus signal to shell
}

} // namespace app
} // namespace touchdown
