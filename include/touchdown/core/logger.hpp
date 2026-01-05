/**
 * @file logger.hpp
 * @brief Logging system for TouchdownOS
 */

#ifndef TOUCHDOWN_CORE_LOGGER_HPP
#define TOUCHDOWN_CORE_LOGGER_HPP

#include <string>
#include <sstream>
#include <systemd/sd-journal.h>

namespace touchdown {

enum class LogLevel {
    DEBUG = LOG_DEBUG,
    INFO = LOG_INFO,
    WARNING = LOG_WARNING,
    ERROR = LOG_ERR,
    CRITICAL = LOG_CRIT
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }
    
    void log(LogLevel level, const std::string& component, const std::string& message) {
        sd_journal_send(
            "PRIORITY=%d", static_cast<int>(level),
            "COMPONENT=%s", component.c_str(),
            "MESSAGE=%s", message.c_str(),
            nullptr
        );
    }
    
    template<typename... Args>
    void debug(const std::string& component, Args&&... args) {
        #ifdef DEBUG_BUILD
        log(LogLevel::DEBUG, component, format(std::forward<Args>(args)...));
        #endif
    }
    
    template<typename... Args>
    void info(const std::string& component, Args&&... args) {
        log(LogLevel::INFO, component, format(std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void warning(const std::string& component, Args&&... args) {
        log(LogLevel::WARNING, component, format(std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void error(const std::string& component, Args&&... args) {
        log(LogLevel::ERROR, component, format(std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    void critical(const std::string& component, Args&&... args) {
        log(LogLevel::CRITICAL, component, format(std::forward<Args>(args)...));
    }
    
private:
    Logger() = default;
    
    template<typename... Args>
    std::string format(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        return oss.str();
    }
};

// Convenience macros (TD_ prefix to avoid conflicts with syslog.h)
#define TD_LOG_DEBUG(...) touchdown::Logger::instance().debug(__VA_ARGS__)
#define TD_LOG_INFO(...) touchdown::Logger::instance().info(__VA_ARGS__)
#define TD_LOG_WARNING(...) touchdown::Logger::instance().warning(__VA_ARGS__)
#define TD_LOG_ERROR(...) touchdown::Logger::instance().error(__VA_ARGS__)
#define TD_LOG_CRITICAL(...) touchdown::Logger::instance().critical(__VA_ARGS__)

} // namespace touchdown

#endif // TOUCHDOWN_CORE_LOGGER_HPP
