/**
 * @file config.cpp
 * @brief Configuration management implementation
 */

#include "touchdown/core/config.hpp"
#include "touchdown/core/logger.hpp"
#include <fstream>
#include <sstream>

namespace touchdown {

Config& Config::instance() {
    static Config config;
    return config;
}

bool Config::load(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        LOG_WARNING("Config", "Failed to open config file: ", config_file);
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            config_data_[key] = value;
        }
    }
    
    LOG_INFO("Config", "Loaded config from: ", config_file);
    return true;
}

bool Config::save(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        LOG_ERROR("Config", "Failed to save config file: ", config_file);
        return false;
    }
    
    for (const auto& [key, value] : config_data_) {
        file << key << "=" << value << "\n";
    }
    
    LOG_INFO("Config", "Saved config to: ", config_file);
    return true;
}

std::string Config::get_string(const std::string& key, const std::string& default_value) {
    auto it = config_data_.find(key);
    return (it != config_data_.end()) ? it->second : default_value;
}

int Config::get_int(const std::string& key, int default_value) {
    auto it = config_data_.find(key);
    if (it != config_data_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_value;
        }
    }
    return default_value;
}

bool Config::get_bool(const std::string& key, bool default_value) {
    auto it = config_data_.find(key);
    if (it != config_data_.end()) {
        return (it->second == "true" || it->second == "1" || it->second == "yes");
    }
    return default_value;
}

void Config::set_string(const std::string& key, const std::string& value) {
    config_data_[key] = value;
}

void Config::set_int(const std::string& key, int value) {
    config_data_[key] = std::to_string(value);
}

void Config::set_bool(const std::string& key, bool value) {
    config_data_[key] = value ? "true" : "false";
}

} // namespace touchdown
