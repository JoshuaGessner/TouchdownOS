/**
 * @file system_info_app.hpp
 * @brief System information display app
 */

#ifndef TOUCHDOWN_APPS_SYSTEM_INFO_APP_HPP
#define TOUCHDOWN_APPS_SYSTEM_INFO_APP_HPP

#include "touchdown/app/app.hpp"

namespace touchdown {
namespace apps {

class SystemInfoApp : public app::TouchdownApp {
public:
    SystemInfoApp(const app::AppMetadata& metadata);
    ~SystemInfoApp() override;
    
    bool init(lv_obj_t* parent) override;
    void show() override;
    void hide() override;
    void update(uint32_t delta_ms) override;
    void cleanup() override;
    
    bool on_button(const ButtonEvent& event) override;
    
private:
    void create_ui();
    void update_info();
    
    std::string get_cpu_usage();
    std::string get_memory_usage();
    std::string get_uptime();
    std::string get_temperature();
    
    lv_obj_t* cpu_label_;
    lv_obj_t* mem_label_;
    lv_obj_t* uptime_label_;
    lv_obj_t* temp_label_;
    
    uint32_t update_timer_;
};

} // namespace apps
} // namespace touchdown

#endif // TOUCHDOWN_APPS_SYSTEM_INFO_APP_HPP
