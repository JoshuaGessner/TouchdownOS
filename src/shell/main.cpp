/**
 * @file main.cpp
 * @brief TouchdownOS Shell main entry point
 */

#include "touchdown/shell/shell.hpp"
#include "touchdown/core/logger.hpp"
#include <csignal>
#include <memory>

static std::unique_ptr<touchdown::shell::Shell> g_shell;

void signal_handler(int signum) {
    if (g_shell) {
        touchdown::LOG_INFO("ShellMain", "Received signal: ", signum);
        g_shell->stop();
    }
}

int main(int argc, char* argv[]) {
    touchdown::LOG_INFO("ShellMain", "Starting TouchdownOS Shell");
    touchdown::LOG_INFO("ShellMain", "Version: 0.1.0");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create and initialize shell
    g_shell = std::make_unique<touchdown::shell::Shell>();
    if (!g_shell->init()) {
        touchdown::LOG_ERROR("ShellMain", "Failed to initialize shell");
        return 1;
    }
    
    // Run shell
    g_shell->run();
    
    touchdown::LOG_INFO("ShellMain", "Shell stopped gracefully");
    return 0;
}
