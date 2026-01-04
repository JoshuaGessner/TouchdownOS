/**
 * @file power_service_main.cpp
 * @brief Power service executable entry point
 */

#include "touchdown/services/power_service.hpp"
#include "touchdown/drivers/display_driver.hpp"
#include "touchdown/core/logger.hpp"
#include <csignal>
#include <memory>

static std::unique_ptr<touchdown::services::PowerService> g_service;

void signal_handler(int signum) {
    if (g_service) {
        touchdown::LOG_INFO("PowerServiceMain", "Received signal: ", signum);
        g_service->stop();
    }
}

int main(int argc, char* argv[]) {
    touchdown::LOG_INFO("PowerServiceMain", "Starting TouchdownOS Power Service");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create display driver instance (for power control)
    auto display = std::make_unique<touchdown::drivers::DisplayDriver>();
    if (!display->init()) {
        touchdown::LOG_ERROR("PowerServiceMain", "Failed to initialize display driver");
        return 1;
    }
    
    // Create and initialize power service
    g_service = std::make_unique<touchdown::services::PowerService>();
    if (!g_service->init(display.get())) {
        touchdown::LOG_ERROR("PowerServiceMain", "Failed to initialize power service");
        return 1;
    }
    
    // Run service
    g_service->run();
    
    touchdown::LOG_INFO("PowerServiceMain", "Power service stopped");
    return 0;
}
