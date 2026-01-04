/**
 * @file input_service_main.cpp
 * @brief Input service executable entry point
 */

#include "touchdown/services/input_service.hpp"
#include "touchdown/drivers/touch_driver.hpp"
#include "touchdown/drivers/button_driver.hpp"
#include "touchdown/core/logger.hpp"
#include <csignal>
#include <memory>

static std::unique_ptr<touchdown::services::InputService> g_service;

void signal_handler(int signum) {
    if (g_service) {
        touchdown::LOG_INFO("InputServiceMain", "Received signal: ", signum);
        g_service->stop();
    }
}

int main(int argc, char* argv[]) {
    touchdown::LOG_INFO("InputServiceMain", "Starting TouchdownOS Input Service");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create driver instances
    auto touch = std::make_unique<touchdown::drivers::TouchDriver>();
    if (!touch->init()) {
        touchdown::LOG_ERROR("InputServiceMain", "Failed to initialize touch driver");
        return 1;
    }
    
    auto button = std::make_unique<touchdown::drivers::ButtonDriver>();
    if (!button->init()) {
        touchdown::LOG_ERROR("InputServiceMain", "Failed to initialize button driver");
        return 1;
    }
    
    // Create and initialize input service
    g_service = std::make_unique<touchdown::services::InputService>();
    if (!g_service->init(touch.get(), button.get())) {
        touchdown::LOG_ERROR("InputServiceMain", "Failed to initialize input service");
        return 1;
    }
    
    // Run service
    g_service->run();
    
    touchdown::LOG_INFO("InputServiceMain", "Input service stopped");
    return 0;
}
