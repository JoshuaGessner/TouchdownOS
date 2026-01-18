/**
 * @file shell.cpp
 * @brief Main shell implementation
 */

#include "touchdown/shell/shell.hpp"
#include "touchdown/shell/theme_engine.hpp"
#include "touchdown/shell/circular_layout.hpp"
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
    : screen_(nullptr)
    , app_container_(nullptr)
    , state_(ShellState::HOME)
    , running_(false)
    , last_time_update_(0)
    , last_update_ms_(0) {
}

Shell::~Shell() {
    stop();
}

bool Shell::init() {
    TD_LOG_INFO("Shell", "Initializing TouchdownOS Shell");
    
    Config::instance().load("/etc/touchdown/shell.conf");
    lv_init();
    
    display_ = std::make_unique<drivers::DisplayDriver>();
    if (!display_->init()) {
        TD_LOG_ERROR("Shell", "Failed to initialize display");
        return false;
    }
    
    touch_ = std::make_unique<drivers::TouchDriver>();
    if (!touch_->init()) {
        TD_LOG_ERROR("Shell", "Failed to initialize touch");
        return false;
    }
    
    button_ = std::make_unique<drivers::ButtonDriver>();
    if (!button_->init()) {
        TD_LOG_ERROR("Shell", "Failed to initialize button");
        return false;
    }
    
    ThemeEngine::instance().init();
    
    screen_ = lv_scr_act();
    CircularLayout::apply_circular_mask(screen_);
    app_container_ = lv_obj_create(screen_);
    lv_obj_set_size(app_container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(app_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(app_container_, 0, 0);
    CircularLayout::apply_circular_mask(app_container_);
    lv_obj_add_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
    
    app_manager_ = std::make_unique<services::AppManager>();
    if (!app_manager_->init()) {
        TD_LOG_ERROR("Shell", "Failed to initialize app manager");
        return false;
    }
    
    home_screen_ = std::make_unique<HomeScreen>();
    home_screen_->create(screen_);
    
    app_launcher_ = std::make_unique<AppLauncher>();
    app_launcher_->create(screen_);
    app_launcher_->set_launch_callback([this](const std::string& app_id) {
        launch_app(app_id);
    });

    // Sample apps (placeholder until manifest-driven launcher is wired)
    app_launcher_->add_app({"settings", "Settings", LV_SYMBOL_SETTINGS, lv_color_hex(0x0088CC)});
    app_launcher_->add_app({"info", "Info", LV_SYMBOL_LIST, lv_color_hex(0x00AA88)});
    app_launcher_->add_app({"power", "Power", LV_SYMBOL_POWER, lv_color_hex(0xCC0044)});
    
    setup_input_handlers();
    go_home();
    
    TD_LOG_INFO("Shell", "Shell initialized successfully");
    return true;
}

void Shell::setup_input_handlers() {
    touch_->set_touch_callback([this](const TouchPoint& p) {
        on_touch(p);
    });
    
    button_->set_button_callback([this](const ButtonEvent& e) {
        on_button(e);
    });
}

void Shell::run() {
    running_ = true;
    sd_notify(0, "READY=1");
    TD_LOG_INFO("Shell", "Shell running");

    last_time_update_ = Utils::get_timestamp_ms();
    last_update_ms_ = last_time_update_;

    uint32_t watchdog_count = 0;
    
    while (running_) {
        uint32_t sleep_ms = lv_timer_handler();

        uint32_t now = Utils::get_timestamp_ms();
        uint32_t delta_ms = now - last_update_ms_;
        last_update_ms_ = now;

        if (now - last_time_update_ >= TIME_UPDATE_INTERVAL_MS) {
            update_time();
            last_time_update_ = now;
        }

        if (app_manager_) {
            app_manager_->update(delta_ms);
        }

        if (++watchdog_count >= 100) {
            sd_notify(0, "WATCHDOG=1");
            watchdog_count = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms < 100 ? sleep_ms : 100));
    }
}

void Shell::stop() {
    running_ = false;
    TD_LOG_INFO("Shell", "Shell stopping");
}

void Shell::on_touch(const TouchPoint& point) {
    TD_LOG_DEBUG("Shell", "Touch: ", static_cast<int>(point.type), " at (", point.x, ",", point.y, ")");

    if (state_ == ShellState::APP_RUNNING && app_manager_) {
        if (app_manager_->handle_touch(point)) {
            return;
        }
    }

    switch (state_) {
        case ShellState::HOME:
            if (point.type == TouchEventType::SWIPE_UP) {
                show_launcher();
            }
            break;
        case ShellState::APP_LAUNCHER:
            if (point.type == TouchEventType::SWIPE_DOWN) {
                go_home();
            }
            break;
        case ShellState::APP_RUNNING:
            // App already received the event above
            break;
    }
}

void Shell::on_button(const ButtonEvent& event) {
    TD_LOG_DEBUG("Shell", "Button: ", static_cast<int>(event.type));

    if (state_ == ShellState::APP_RUNNING && app_manager_) {
        if (app_manager_->handle_button(event)) {
            return;
        }
    }

    switch (event.type) {
        case ButtonEventType::SINGLE_PRESS:
            if (state_ == ShellState::APP_RUNNING) {
                auto* active = app_manager_ ? app_manager_->get_active_app() : nullptr;
                if (active) {
                    std::string app_id = active->get_metadata().id;
                    app_manager_->terminate_app(app_id);
                }
                go_home();
            } else {
                show_launcher();
            }
            break;

        case ButtonEventType::DOUBLE_PRESS:
            if (state_ == ShellState::HOME) {
                show_launcher();
            } else {
                go_home();
            }
            break;

        default:
            break;
    }
}

void Shell::go_home() {
    change_state(ShellState::HOME);

    if (app_manager_) {
        auto* active = app_manager_->get_active_app();
        if (active) {
            app_manager_->terminate_app(active->get_metadata().id);
        }
    }

    if (app_container_) {
        lv_obj_add_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
    }
    if (app_launcher_) {
        app_launcher_->hide();
    }
    if (home_screen_) {
        home_screen_->show();
    }
}

void Shell::show_launcher() {
    change_state(ShellState::APP_LAUNCHER);

    if (home_screen_) {
        home_screen_->hide();
    }
    if (app_container_) {
        lv_obj_add_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
    }
    if (app_launcher_) {
        app_launcher_->animate_show();
    }
}

void Shell::launch_app(const std::string& app_id) {
    if (!app_manager_) {
        TD_LOG_ERROR("Shell", "App manager not initialized");
        return;
    }

    if (app_manager_->launch_app(app_id, app_container_)) {
        change_state(ShellState::APP_RUNNING);
        if (app_launcher_) {
            app_launcher_->animate_hide();
        }
        if (home_screen_) {
            home_screen_->hide();
        }
        if (app_container_) {
            lv_obj_clear_flag(app_container_, LV_OBJ_FLAG_HIDDEN);
        }
        TD_LOG_INFO("Shell", "App launched successfully: ", app_id);
    } else {
        TD_LOG_ERROR("Shell", "Failed to launch app: ", app_id);
    }
}

void Shell::change_state(ShellState new_state) {
    state_ = new_state;
}

void Shell::update_time() {
    if (home_screen_) {
        home_screen_->update_time();
    }
}

} // namespace shell
} // namespace touchdown
