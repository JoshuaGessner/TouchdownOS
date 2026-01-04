/**
 * @file shell.hpp
 * @brief Main TouchdownOS shell coordinator
 */

#ifndef TOUCHDOWN_SHELL_SHELL_HPP
#define TOUCHDOWN_SHELL_SHELL_HPP

#include "touchdown/drivers/display_driver.hpp"
#include "touchdown/drivers/touch_driver.hpp"
#include "touchdown/drivers/button_driver.hpp"
#include "touchdown/shell/home_screen.hpp"
#include "touchdown/shell/app_launcher.hpp"
#include <memory>
#include <atomic>

namespace touchdown {
namespace shell {

enum class ShellState {
    HOME,
    APP_LAUNCHER,
    APP_RUNNING
};

class Shell {
public:
    Shell();
    ~Shell();
    
    /**
     * @brief Initialize shell
     */
    bool init();
    
    /**
     * @brief Main shell loop
     */
    void run();
    
    /**
     * @brief Stop shell
     */
    void stop();
    
    /**
     * @brief Navigate to home screen
     */
    void go_home();
    
    /**
     * @brief Show app launcher
     */
    void show_launcher();
    
    /**
     * @brief Launch an app
     */
    void launch_app(const std::string& app_id);
    
private:
    void setup_input_handlers();
    void on_touch(const TouchPoint& point);
    void on_button(const ButtonEvent& event);
    void change_state(ShellState new_state);
    void update_time();
    
    // Hardware drivers
    std::unique_ptr<drivers::DisplayDriver> display_;
    std::unique_ptr<drivers::TouchDriver> touch_;
    std::unique_ptr<drivers::ButtonDriver> button_;
    
    // UI components
    lv_obj_t* screen_;
    std::unique_ptr<HomeScreen> home_screen_;
    std::unique_ptr<AppLauncher> app_launcher_;
    
    // State
    ShellState state_;
    std::atomic<bool> running_;
    uint32_t last_time_update_;
};

} // namespace shell
} // namespace touchdown

#endif // TOUCHDOWN_SHELL_SHELL_HPP
