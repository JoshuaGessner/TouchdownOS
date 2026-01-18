/**
 * @file display_driver.cpp
 * @brief DRM/KMS display driver implementation
 */

#include "touchdown/drivers/display_driver.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/utils.hpp"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>

namespace touchdown {
namespace drivers {

class DisplayDriver::Impl {
public:
    int drm_fd = -1;
    uint32_t connector_id = 0;
    uint32_t crtc_id = 0;
    uint32_t fb_id = 0;
    uint32_t width = DisplayConfig::WIDTH;
    uint32_t height = DisplayConfig::HEIGHT;
    
    void* fb_base = nullptr;
    size_t fb_size = 0;
    
    drmModeModeInfo mode;
    drmModeCrtc* saved_crtc = nullptr;
};

DisplayDriver::DisplayDriver() : impl_(std::make_unique<Impl>()), display_(nullptr) {
}

DisplayDriver::~DisplayDriver() {
    deinit();
}

bool DisplayDriver::init(const std::string& device) {
    TD_LOG_INFO("DisplayDriver", "Initializing DRM display: ", device);
    
    // Open DRM device
    impl_->drm_fd = open(device.c_str(), O_RDWR | O_CLOEXEC);
    if (impl_->drm_fd < 0) {
        TD_LOG_ERROR("DisplayDriver", "Failed to open DRM device: ", device);
        return false;
    }
    
    // Get DRM resources
    drmModeRes* resources = drmModeGetResources(impl_->drm_fd);
    if (!resources) {
        TD_LOG_ERROR("DisplayDriver", "Failed to get DRM resources");
        close(impl_->drm_fd);
        return false;
    }
    
    // Find first connected connector
    drmModeConnector* connector = nullptr;
    for (int i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(impl_->drm_fd, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
            impl_->connector_id = connector->connector_id;
            impl_->mode = connector->modes[0];  // Use first mode
            break;
        }
        drmModeFreeConnector(connector);
        connector = nullptr;
    }
    
    if (!connector) {
        TD_LOG_ERROR("DisplayDriver", "No connected display found");
        drmModeFreeResources(resources);
        close(impl_->drm_fd);
        return false;
    }
    
    // Find encoder and CRTC
    drmModeEncoder* encoder = drmModeGetEncoder(impl_->drm_fd, connector->encoder_id);
    if (encoder) {
        impl_->crtc_id = encoder->crtc_id;
        drmModeFreeEncoder(encoder);
    } else {
        // Find first available CRTC
        for (int i = 0; i < resources->count_crtcs; i++) {
            impl_->crtc_id = resources->crtcs[i];
            break;
        }
    }
    
    impl_->saved_crtc = drmModeGetCrtc(impl_->drm_fd, impl_->crtc_id);
    
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    
    // Create dumb buffer
    struct drm_mode_create_dumb create_dumb = {};
    create_dumb.width = impl_->width;
    create_dumb.height = impl_->height;
    create_dumb.bpp = 16;  // RGB565
    
    if (drmIoctl(impl_->drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0) {
        TD_LOG_ERROR("DisplayDriver", "Failed to create dumb buffer");
        close(impl_->drm_fd);
        return false;
    }
    
    // Create framebuffer
    uint32_t handles[4] = {create_dumb.handle, 0, 0, 0};
    uint32_t pitches[4] = {create_dumb.pitch, 0, 0, 0};
    uint32_t offsets[4] = {0, 0, 0, 0};
    
    if (drmModeAddFB2(impl_->drm_fd, impl_->width, impl_->height, DRM_FORMAT_RGB565,
                      handles, pitches, offsets, &impl_->fb_id, 0) < 0) {
        TD_LOG_ERROR("DisplayDriver", "Failed to create framebuffer");
        close(impl_->drm_fd);
        return false;
    }
    
    // Map framebuffer
    struct drm_mode_map_dumb map_dumb = {};
    map_dumb.handle = create_dumb.handle;
    
    if (drmIoctl(impl_->drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        TD_LOG_ERROR("DisplayDriver", "Failed to map dumb buffer");
        close(impl_->drm_fd);
        return false;
    }
    
    impl_->fb_size = create_dumb.size;
    impl_->fb_base = mmap(0, impl_->fb_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                          impl_->drm_fd, map_dumb.offset);
    
    if (impl_->fb_base == MAP_FAILED) {
        TD_LOG_ERROR("DisplayDriver", "Failed to mmap framebuffer");
        close(impl_->drm_fd);
        return false;
    }
    
    // Set mode
    if (drmModeSetCrtc(impl_->drm_fd, impl_->crtc_id, impl_->fb_id, 0, 0,
                       &impl_->connector_id, 1, &impl_->mode) < 0) {
        TD_LOG_ERROR("DisplayDriver", "Failed to set CRTC mode");
        munmap(impl_->fb_base, impl_->fb_size);
        close(impl_->drm_fd);
        return false;
    }
    
    // Initialize LVGL display
    display_ = lv_display_create(impl_->width, impl_->height);
    if (!display_) {
        TD_LOG_ERROR("DisplayDriver", "Failed to create LVGL display");
        return false;
    }
    
    lv_display_set_flush_cb(display_, flush_cb);
    lv_display_set_user_data(display_, this);
    
    // Clear framebuffer
    std::memset(impl_->fb_base, 0, impl_->fb_size);
    
    TD_LOG_INFO("DisplayDriver", "Display initialized: ", impl_->width, "x", impl_->height);
    return true;
}

void DisplayDriver::deinit() {
    if (impl_->fb_base && impl_->fb_base != MAP_FAILED) {
        munmap(impl_->fb_base, impl_->fb_size);
        impl_->fb_base = nullptr;
    }
    
    if (impl_->saved_crtc && impl_->drm_fd >= 0) {
        drmModeSetCrtc(impl_->drm_fd, impl_->saved_crtc->crtc_id, impl_->saved_crtc->buffer_id,
                       impl_->saved_crtc->x, impl_->saved_crtc->y,
                       &impl_->connector_id, 1, &impl_->saved_crtc->mode);
        drmModeFreeCrtc(impl_->saved_crtc);
    }
    
    if (impl_->fb_id && impl_->drm_fd >= 0) {
        drmModeRmFB(impl_->drm_fd, impl_->fb_id);
    }
    
    if (impl_->drm_fd >= 0) {
        close(impl_->drm_fd);
        impl_->drm_fd = -1;
    }
    
    TD_LOG_INFO("DisplayDriver", "Display deinitialized");
}

void DisplayDriver::flush_cb(lv_display_t* disp, const lv_area_t* area, unsigned char* color_p) {
    DisplayDriver* driver = static_cast<DisplayDriver*>(lv_display_get_user_data(disp));
    driver->flush_display(area, color_p);
}

void DisplayDriver::flush_display(const lv_area_t* area, unsigned char* color_p) {
    if (!impl_->fb_base) return;
    
    uint16_t* fb = static_cast<uint16_t*>(impl_->fb_base);
    uint16_t* src = reinterpret_cast<uint16_t*>(color_p);
    
    int32_t width = area->x2 - area->x1 + 1;
    int32_t height = area->y2 - area->y1 + 1;
    
    for (int32_t y = 0; y < height; y++) {
        int32_t fb_y = area->y1 + y;
        int32_t fb_offset = fb_y * impl_->width + area->x1;
        std::memcpy(&fb[fb_offset], &src[y * width], width * sizeof(uint16_t));
    }
    
    lv_display_flush_ready(display_);
}

void DisplayDriver::set_brightness(uint8_t brightness) {
    // Brightness control via backlight sysfs
    std::string path = "/sys/class/backlight/";
    // Implementation depends on specific backlight device
    TD_LOG_DEBUG("DisplayDriver", "Set brightness: ", (int)brightness);
}

void DisplayDriver::set_power(bool on) {
    if (impl_->drm_fd < 0) return;
    
    // Use DRM DPMS for power management
    uint32_t dpms_value = on ? DRM_MODE_DPMS_ON : DRM_MODE_DPMS_OFF;
    
    drmModeConnector* connector = drmModeGetConnector(impl_->drm_fd, impl_->connector_id);
    if (connector) {
        for (int i = 0; i < connector->count_props; i++) {
            drmModePropertyRes* prop = drmModeGetProperty(impl_->drm_fd, connector->props[i]);
            if (prop && strcmp(prop->name, "DPMS") == 0) {
                drmModeConnectorSetProperty(impl_->drm_fd, impl_->connector_id, prop->prop_id, dpms_value);
                drmModeFreeProperty(prop);
                break;
            }
            if (prop) drmModeFreeProperty(prop);
        }
        drmModeFreeConnector(connector);
    }
    
    TD_LOG_INFO("DisplayDriver", "Display power: ", on ? "ON" : "OFF");
}

bool DisplayDriver::is_point_safe(int16_t x, int16_t y) {
    return Utils::is_point_in_circle(x, y, DisplayConfig::CENTER_X, DisplayConfig::CENTER_Y, DisplayConfig::SAFE_RADIUS);
}

} // namespace drivers
} // namespace touchdown
