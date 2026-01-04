/**
 * @file shell.cpp
 * @brief Main shell implementation
 */

#include "touchdown/shell/shell.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/core/logger.hpp"
#include "touchdown/core/utils.hpp"
#include "touchdown/core/config.hpp"
#include <systemd/sd-daemon.h>
#include <thread>
#include <chrono>

namespace touchdown {
namespace shell {

constexpr uint32_t TIME_UPDATE_INTERVAL_MS = 1000;  // Update time every second

Shell::Shell()
    : state_(ShellState::HOME)
    , running_(false)
    , last_time_update_(0)
    , last_update_ms_(0) {
}

Shell::~Shell() {
    stop();
}

bool Shell::init() {
    LOG_INFO("Shell", "Initializing TouchdownOS Shell");
    
    // Load configuration
    Config::instance().load("/etc/touchdown/shell.conf");
    
    // Initialize LVGL
    lv_init();
    
    // Initialize display driver
    display_ = std::make_unique<drivers::DisplayDriver>();
    if (!display_->init()) {
        LOG_ERROR("Shell", "Failed to initialize display");
        return false;
    }
    
    // Initialize touch driver
    touch_ = std::make_unique<drivers::TouchDriver>();
    if (!touch_->init()) {
        LOG_ERROR("Shell", "Failed to initialize touch");
        return false;
    }
    
    // Initialize button driver
    button_ = std::make_unique<drivers::ButtonDriver>();
    if (!button_->init()) {
        LOG_ERROR("Shell", "Failed to initialize button");
        return false;
    }
    
    // Initialize theme
    ThemeEngine::instance().init();
    
    // Create main screen
    screen_ = lv_scr_act();
    app container
    app_container_ = lv_obj_create(screen_);
    lv_obj_set_size(app_container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(app_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(app_container_, 0, 0);
    lv_obj_add_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
    
    // Initialize app manager
    app_manager_ = std::make_unique<services::AppManager>();
    if (!app_manager_->init()) {
        LOG_ERROR("Shell", "Failed to initialize app manager");
        return false;
    }
    
    // Create 
    // Create UI components
    home_screen_ = std::make_unique<HomeScreen>();
    home_screen_->create(screen_);
    
    app_launcher_ = std::make_unique<AppLauncher>();
    app_launcher_->create(screen_);
    
    // Setup input handlers
    setup_input_handlers();
    
    // Add sample apps
    app_launcher_->add_app({"settings", "Settings", LV_SYMBOL_SETTINGS, lv_color_hex(0x0088CC)});
    app_launcher_->add_app({"info", "Info", LV_SYMBOL_LIST, lv_color_hex(0x00AA88)});
    app_launcher_->add_app({"power", "Power", LV_SYMBOL_POWER, lv_color_hex(0xCC0044)});
    
    app_launcher_->set_launch_callback([this](const std::string& app_id) {
        launch_app(app_id);
    });
    
    // Start on home screen
    go_home();
    
    LOG_INFO("Shell", "Shell initialized successfully");
    return true;
}

void Shell::setup_input_handlers() {
    // Register touch callback
    touch_->set_touch_callback([this](const TouchPoint& p) {
        on_touch(p);
    });
    
    // Register button callback
    button_->set_button_callback([this](const ButtonEvent& e) {
        on_button(e);
    });
}
uint32_t now = Utils::get_timestamp_ms();
        
        // Update time periodically
        if (now - last_time_update_ >= TIME_UPDATE_INTERVAL_MS) {
            update_time();
            last_time_update_ = now;
        }
        
        // Update app manager and active apps
        if (app_manager_) {
            uint32_t delta_ms = last_update_ms_ > 0 ? now - last_update_ms_ : 0;
            app_manager_->update(delta_ms);
            last_update_msready
    sd_notify(0, "READY=1");
    LOG_INFO("Shell", "Shell running");
    
    uint32_t watchdog_count = 0;
    
    while (running_) {
        // Handle LVGL tasks
        uint32_t sleep_ms = lv_timer_handler();
        
        // Update time periodically
        uint32_t now = Utils::get_timestamp_ms();
        if (now - last_time_update_ >= TIME_UPDATE_INTERVAL_MS) {
            update_time();
            last_time_update_ = now;
        }
        
        // Send watchdog keepalive every 10 seconds
        if (++watchdog_count >= 100) {
            sd_notify(0, "WATCHDOG=1");
            watchdog_count = 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(
            sleep_ms < 100 ? sleep_ms : 100));
    }
}

void Shell::stop() {
    running_ = false;
    LOG_INFO("Shell", "Shell stopping");
}

void Shell::on_touch(const TouchPoint& point) {
    // Notify power service of activity (reset idle timer)
    // This would normally go through D-Bus
    
    LOG_DEBUG("Shell", "Touch: ", static_cast<int>(point.type), " at (", point.x, ",", point.y, ")");
    
    // Handle gestures based on state
    switch (state_) {
        case ShellState::HOME:
            if (point.type == TouchEventType::SWIPE_UP) {
                show_launcher();
            }
            break;
            Forward input to active app
            if (app_manager_) {
                if (point.type == TouchEventType::SWIPE_DOWN) {
                    // Back gesture - close app and go home
                    auto* active = app_manager_->get_active_app();
                    if (active) {
                        if (!active->on_back()) {
                            // App didn't handle back - close it
    // Forward to app if running
    if (state_ == ShellState::APP_RUNNING && app_manager_) {
        if (app_manager_->handle_button(event)) {
            return;  // App handled the button
        }
    }
    
    switch (event.type) {
        case ButtonEventType::SINGLE_PRESS:
            if (state_ == ShellState::APP_RUNNING) {
                // Back from app
                auto* active = app_manager_->get_active_app();
                if (active) {
                    std::string app_id = active->get_metadata().id;
                    app_manager_->terminate_app(app_id);
                }
                go_home();
            } else {
                // Toggle display power via D-Bus call to power service
                LOG_INFO("Shell", "Toggle display power");
            }
            break;
            
        case ButtonEventType::DOUBLE_PRESS:
            // User-configurable action
            if (state_ == ShellState::HOME) {
                show_launcher();
            } else if (state_ == ShellState::APP_LAUNCHER) {
                go_home();
            } else {
                // In app - go home
                auto* active = app_manager_->get_active_app();
                if (active) {
                    std::string app_id = active->get_metadata().id;
                    app_manager_->terminate_app(app_id);
                }t.type == TouchEventType::SWIPE_DOWN) {
                go_home();
            }
            break;
            
        case ShellState::APP_RUNNING:
    // Hide app container
    lv_obj_add_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
    
            // App handles its own input
            break;
    }
}

voidif (!app_manager_) {
        LOG_ERROR("Shell", "App manager not initialized");
        return;
    }
    
    // Launch app via app manager
    if (app_manager_->launch_app(app_id, app_container_)) {
        change_state(ShellState::APP_RUNNING);
        app_launcher_->animate_hide();
        home_screen_->hide();
        
        // Show app container
        lv_obj_clear_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
        
        LOG_INFO("Shell", "App launched successfully: ", app_id);
    } else {
        LOG_ERROR("Shell", "Failed to launch app: ", app_id);
    }
            LOG_INFO("Shell", "Toggle display power");
            break;
            
        case ButtonEventType::DOUBLE_PRESS:
            // User-configurable action
            if (state_ == ShellState::HOME) {
                show_launcher();
            } else {
                go_home();
            }
            break;
            
        case ButtonEventType::LONG_PRESS:
            // Power menu or shutdown
            LOG_INFO("Shell", "Show power menu");
            break;
            
        default:
            break;
    }
}

void Shell::change_state(ShellState new_state) {
    if (state_ == new_state) return;
    
    LOG_INFO("Shell", "State change: ", static_cast<int>(state_), " -> ", static_cast<int>(new_state));
    state_ = new_state;
}

void Shell::go_home() {
    change_state(ShellState::HOME);
    home_screen_->show();
    app_launcher_->animate_hide();
}

void Shell::show_launcher() {
    change_state(ShellState::APP_LAUNCHER);
    app_launcher_->animate_show();
}

void Shell::launch_app(const std::string& app_id) {
    LOG_INFO("Shell", "Launching app: ", app_id);
    
    change_state(ShellState::APP_RUNNING);
    app_launcher_->animate_hide();
    
    // TODO: Actually launch the app via app manager service
    // For now, just go back home after a delay
    // In real implementation, this would communicate with the app manager
}

void Shell::update_time() {
    if (state_ == ShellState::HOME) {
        home_screen_->update_time();
        
        // TODO: Update battery and network info from services
        // This would normally query via D-Bus
        BatteryInfo battery{100, false, 4200, 0};
        home_screen_->update_battery(battery);
        
        NetworkInfo network{NetworkStatus::CONNECTED, NetworkStatus::DISCONNECTED, "MyWiFi", -50};
        home_screen_->update_network(network);
    }
}

} // namespace shell
} // namespace touchdown
