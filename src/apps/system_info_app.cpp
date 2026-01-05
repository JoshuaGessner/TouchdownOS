/**
 * @file system_info_app.cpp
 * @brief System information app implementation
 */

#include "touchdown/apps/system_info_app.hpp"
#include "touchdown/app/app_registry.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/core/logger.hpp"
#include <fstream>
#include <sstream>

namespace touchdown {
namespace apps {

constexpr uint32_t UPDATE_INTERVAL_MS = 2000;  // Update every 2 seconds

SystemInfoApp::SystemInfoApp(const app::AppMetadata& metadata)
    : TouchdownApp(metadata)
    , cpu_label_(nullptr)
    , mem_label_(nullptr)
    , uptime_label_(nullptr)
    , temp_label_(nullptr)
    , update_timer_(0) {
}

SystemInfoApp::~SystemInfoApp() {
}

bool SystemInfoApp::init(lv_obj_t* parent) {
    TD_LOG_INFO("SystemInfoApp", "Initializing system info app");
    
    create_container(parent);
    
    auto& theme = shell::ThemeEngine::instance();
    lv_obj_set_style_bg_color(container_, theme.get_palette().background, 0);
    
    create_ui();
    update_info();
    
    return true;
}

void SystemInfoApp::create_ui() {
    auto& theme = shell::ThemeEngine::instance();
    
    // Title
    lv_obj_t* title = lv_label_create(container_);
    lv_label_set_text(title, "System Info");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title, theme.get_palette().text_primary, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);
    
    // Info labels
    int y_offset = 50;
    int y_step = 30;
    
    cpu_label_ = lv_label_create(container_);
    lv_label_set_text(cpu_label_, "CPU: --");
    lv_obj_set_style_text_color(cpu_label_, theme.get_palette().text_secondary, 0);
    lv_obj_align(cpu_label_, LV_ALIGN_TOP_LEFT, 30, y_offset);
    
    mem_label_ = lv_label_create(container_);
    lv_label_set_text(mem_label_, "Memory: --");
    lv_obj_set_style_text_color(mem_label_, theme.get_palette().text_secondary, 0);
    lv_obj_align(mem_label_, LV_ALIGN_TOP_LEFT, 30, y_offset + y_step);
    
    uptime_label_ = lv_label_create(container_);
    lv_label_set_text(uptime_label_, "Uptime: --");
    lv_obj_set_style_text_color(uptime_label_, theme.get_palette().text_secondary, 0);
    lv_obj_align(uptime_label_, LV_ALIGN_TOP_LEFT, 30, y_offset + y_step * 2);
    
    temp_label_ = lv_label_create(container_);
    lv_label_set_text(temp_label_, "Temp: --");
    lv_obj_set_style_text_color(temp_label_, theme.get_palette().text_secondary, 0);
    lv_obj_align(temp_label_, LV_ALIGN_TOP_LEFT, 30, y_offset + y_step * 3);
    
    // Instructions
    lv_obj_t* help = lv_label_create(container_);
    lv_label_set_text(help, "Press button to exit");
    lv_obj_set_style_text_color(help, theme.get_palette().text_secondary, 0);
    lv_obj_set_style_text_font(help, &lv_font_montserrat_12, 0);
    lv_obj_align(help, LV_ALIGN_BOTTOM_MID, 0, -15);
}

void SystemInfoApp::update_info() {
    if (cpu_label_) {
        std::string cpu = "CPU: " + get_cpu_usage();
        lv_label_set_text(cpu_label_, cpu.c_str());
    }
    
    if (mem_label_) {
        std::string mem = "Memory: " + get_memory_usage();
        lv_label_set_text(mem_label_, mem.c_str());
    }
    
    if (uptime_label_) {
        std::string uptime = "Uptime: " + get_uptime();
        lv_label_set_text(uptime_label_, uptime.c_str());
    }
    
    if (temp_label_) {
        std::string temp = "Temp: " + get_temperature();
        lv_label_set_text(temp_label_, temp.c_str());
    }
}

std::string SystemInfoApp::get_cpu_usage() {
    // Read /proc/stat to calculate CPU usage
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return "N/A";
    
    std::string line;
    std::getline(file, line);
    
    // Simple parsing - in real implementation, calculate delta
    return "~25%";  // Placeholder
}

std::string SystemInfoApp::get_memory_usage() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return "N/A";
    
    std::string line;
    uint64_t total = 0, available = 0;
    
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label >> total;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line);
            std::string label;
            iss >> label >> available;
        }
    }
    
    if (total > 0) {
        uint64_t used = total - available;
        int percent = (used * 100) / total;
        return std::to_string(percent) + "%";
    }
    
    return "N/A";
}

std::string SystemInfoApp::get_uptime() {
    std::ifstream file("/proc/uptime");
    if (!file.is_open()) return "N/A";
    
    double uptime_seconds;
    file >> uptime_seconds;
    
    int hours = static_cast<int>(uptime_seconds) / 3600;
    int minutes = (static_cast<int>(uptime_seconds) % 3600) / 60;
    
    return std::to_string(hours) + "h " + std::to_string(minutes) + "m";
}

std::string SystemInfoApp::get_temperature() {
    // Raspberry Pi thermal zone
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    if (!file.is_open()) return "N/A";
    
    int temp_millidegrees;
    file >> temp_millidegrees;
    
    int temp_celsius = temp_millidegrees / 1000;
    return std::to_string(temp_celsius) + "°C";
}

void SystemInfoApp::show() {
    visible_ = true;
    if (container_) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
    update_timer_ = 0;
}

void SystemInfoApp::hide() {
    visible_ = false;
    if (container_) {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void SystemInfoApp::update(uint32_t delta_ms) {
    update_timer_ += delta_ms;
    
    if (update_timer_ >= UPDATE_INTERVAL_MS) {
        update_info();
        update_timer_ = 0;
    }
}

void SystemInfoApp::cleanup() {
    TD_LOG_INFO("SystemInfoApp", "Cleanup");
}

bool SystemInfoApp::on_button(const ButtonEvent& event) {
    if (event.type == ButtonEventType::SINGLE_PRESS) {
        request_close();
        return true;
    }
    return false;
}

} // namespace apps
} // namespace touchdown

// Register the app
namespace touchdown {
namespace apps {
REGISTER_APP(SystemInfoApp, "info")
} // namespace apps
} // namespace touchdown
