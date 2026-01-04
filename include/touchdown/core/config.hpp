/**
 * @file config.hpp
 * @brief Configuration management for TouchdownOS
 */

#ifndef TOUCHDOWN_CORE_CONFIG_HPP
#define TOUCHDOWN_CORE_CONFIG_HPP

#include <string>
#include <map>
#include <memory>

namespace touchdown {

class Config {
public:
    static Config& instance();
    
    bool load(const std::string& config_file);
    bool save(const std::string& config_file);
    
    std::string get_string(const std::string& key, const std::string& default_value = "");
    int get_int(const std::string& key, int default_value = 0);
    bool get_bool(const std::string& key, bool default_value = false);
    
    void set_string(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int value);
    void set_bool(const std::string& key, bool value);
    
private:
    Config() = default;
    std::map<std::string, std::string> config_data_;
};

} // namespace touchdown

#endif // TOUCHDOWN_CORE_CONFIG_HPP
