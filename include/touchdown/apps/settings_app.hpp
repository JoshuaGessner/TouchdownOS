/**
 * @file settings_app.hpp
 * @brief Settings application for TouchdownOS
 */

#ifndef TOUCHDOWN_APPS_SETTINGS_APP_HPP
#define TOUCHDOWN_APPS_SETTINGS_APP_HPP

#include "touchdown/app/app.hpp"
#include <vector>

namespace touchdown {
namespace apps {

struct SettingItem {
    std::string label;
    std::string value;
    std::function<void()> on_click;
};

class SettingsApp : public app::TouchdownApp {
public:
    SettingsApp(const app::AppMetadata& metadata);
    ~SettingsApp() override;
    
    bool init(lv_obj_t* parent) override;
    void show() override;
    void hide() override;
    void cleanup() override;
    
    bool on_touch(const TouchPoint& point) override;
    bool on_button(const ButtonEvent& event) override;
    
private:
    void create_ui();
    void create_setting_item(const SettingItem& item, int index);
    
    void on_theme_toggle();
    void on_brightness_adjust();
    void on_about();
    
    lv_obj_t* list_;
    std::vector<lv_obj_t*> items_;
    lv_style_t item_style_;
};

} // namespace apps
} // namespace touchdown

#endif // TOUCHDOWN_APPS_SETTINGS_APP_HPP
