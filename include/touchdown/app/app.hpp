/**
 * @file app.hpp
 * @brief Base class for TouchdownOS applications
 */

#ifndef TOUCHDOWN_APP_APP_HPP
#define TOUCHDOWN_APP_APP_HPP

#include "touchdown/core/types.hpp"
#include "lvgl.h"
#include <string>
#include <memory>

namespace touchdown {
namespace app {

/**
 * @brief Application metadata
 */
struct AppMetadata {
    std::string id;              // Unique app ID (e.g., "com.touchdown.settings")
    std::string name;            // Display name
    std::string version;         // Version string
    std::string description;     // App description
    std::string icon;            // Icon symbol or path
    lv_color_t color;           // App color theme
    std::vector<std::string> permissions;  // Required permissions
};

/**
 * @brief Base class for all TouchdownOS applications
 * 
 * Apps should inherit from this class and implement all virtual methods.
 * The app lifecycle is managed by the AppManager service.
 */
class TouchdownApp {
public:
    TouchdownApp(const AppMetadata& metadata);
    virtual ~TouchdownApp();
    
    /**
     * @brief Initialize app resources
     * @param parent LVGL parent object to create UI in
     * @return true on success
     */
    virtual bool init(lv_obj_t* parent) = 0;
    
    /**
     * @brief Show the app (make visible)
     */
    virtual void show() = 0;
    
    /**
     * @brief Hide the app (background/minimized)
     */
    virtual void hide() = 0;
    
    /**
     * @brief Pause the app (save state, stop animations)
     */
    virtual void pause() {
        // Default: just hide
        hide();
    }
    
    /**
     * @brief Resume the app from pause
     */
    virtual void resume() {
        // Default: just show
        show();
    }
    
    /**
     * @brief Update app state (called regularly when visible)
     * @param delta_ms Time since last update in milliseconds
     */
    virtual void update(uint32_t delta_ms) {
        // Default: no-op
    }
    
    /**
     * @brief Handle touch input
     * @param point Touch event data
     * @return true if handled, false to propagate
     */
    virtual bool on_touch(const TouchPoint& point) {
        return false;  // Not handled by default
    }
    
    /**
     * @brief Handle button input
     * @param event Button event data
     * @return true if handled, false to propagate
     */
    virtual bool on_button(const ButtonEvent& event) {
        return false;  // Not handled by default
    }
    
    /**
     * @brief Handle back gesture/button (return to home)
     * @return true if handled (stay in app), false to go back
     */
    virtual bool on_back() {
        return false;  // Go back by default
    }
    
    /**
     * @brief Clean up app resources
     */
    virtual void cleanup() = 0;
    
    /**
     * @brief Get app metadata
     */
    const AppMetadata& get_metadata() const { return metadata_; }
    
    /**
     * @brief Get root container
     */
    lv_obj_t* get_container() const { return container_; }
    
    /**
     * @brief Check if app is visible
     */
    bool is_visible() const { return visible_; }
    
    /**
     * @brief Check if app is paused
     */
    bool is_paused() const { return paused_; }
    
protected:
    /**
     * @brief Create the root container for the app
     */
    lv_obj_t* create_container(lv_obj_t* parent);
    
    /**
     * @brief Request permission (checks if granted)
     */
    bool has_permission(const std::string& permission) const;
    
    /**
     * @brief Request to close the app
     */
    void request_close();
    
    AppMetadata metadata_;
    lv_obj_t* container_;
    bool visible_;
    bool paused_;
};

/**
 * @brief App factory function type
 */
using AppFactory = std::function<std::unique_ptr<TouchdownApp>(const AppMetadata&)>;

/**
 * @brief Macro to register an app
 */
#define REGISTER_APP(AppClass, AppId) \
    static bool AppClass##_registered = \
        touchdown::app::AppRegistry::instance().register_app( \
            AppId, \
            [](const touchdown::app::AppMetadata& meta) -> std::unique_ptr<touchdown::app::TouchdownApp> { \
                return std::make_unique<AppClass>(meta); \
            } \
        );

} // namespace app
} // namespace touchdown

#endif // TOUCHDOWN_APP_APP_HPP
