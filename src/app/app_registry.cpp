/**
 * @file app_registry.cpp
 * @brief App registry implementation
 */

#include "touchdown/app/app_registry.hpp"
#include "touchdown/core/logger.hpp"
#include <fstream>
#include <filesystem>
#include <json/json.h>

namespace fs = std::filesystem;

namespace touchdown {
namespace app {

AppRegistry& AppRegistry::instance() {
    static AppRegistry registry;
    return registry;
}

bool AppRegistry::register_app(const std::string& app_id, AppFactory factory) {
    if (factories_.find(app_id) != factories_.end()) {
        LOG_WARNING("AppRegistry", "App already registered: ", app_id);
        return false;
    }
    
    factories_[app_id] = factory;
    LOG_INFO("AppRegistry", "Registered app: ", app_id);
    return true;
}

std::unique_ptr<TouchdownApp> AppRegistry::create_app(const std::string& app_id,
                                                      const AppMetadata& metadata) {
    auto it = factories_.find(app_id);
    if (it == factories_.end()) {
        LOG_ERROR("AppRegistry", "App not registered: ", app_id);
        return nullptr;
    }
    
    try {
        auto app = it->second(metadata);
        LOG_INFO("AppRegistry", "Created app instance: ", app_id);
        return app;
    } catch (const std::exception& e) {
        LOG_ERROR("AppRegistry", "Failed to create app: ", app_id, " - ", e.what());
        return nullptr;
    }
}

std::vector<std::string> AppRegistry::get_app_ids() const {
    std::vector<std::string> ids;
    ids.reserve(factories_.size());
    
    for (const auto& [id, _] : factories_) {
        ids.push_back(id);
    }
    
    return ids;
}

bool AppRegistry::has_app(const std::string& app_id) const {
    return factories_.find(app_id) != factories_.end();
}

AppMetadata AppRegistry::load_metadata(const std::string& manifest_path) {
    AppMetadata metadata;
    
    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        LOG_ERROR("AppRegistry", "Failed to open manifest: ", manifest_path);
        return metadata;
    }
    
    // Simple JSON parsing (in real implementation, use nlohmann/json or similar)
    // For now, a basic implementation
    std::string line;
    while (std::getline(file, line)) {
        // Very basic key-value parsing
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace and quotes
            key.erase(0, key.find_first_not_of(" \t\""));
            key.erase(key.find_last_not_of(" \t\",") + 1);
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\",") + 1);
            
            if (key == "id") metadata.id = value;
            else if (key == "name") metadata.name = value;
            else if (key == "version") metadata.version = value;
            else if (key == "description") metadata.description = value;
            else if (key == "icon") metadata.icon = value;
        }
    }
    
    LOG_INFO("AppRegistry", "Loaded metadata for: ", metadata.id);
    return metadata;
}

std::vector<AppMetadata> AppRegistry::scan_apps(const std::string& apps_dir) {
    std::vector<AppMetadata> apps;
    
    if (!fs::exists(apps_dir)) {
        LOG_WARNING("AppRegistry", "Apps directory not found: ", apps_dir);
        return apps;
    }
    
    for (const auto& entry : fs::directory_iterator(apps_dir)) {
        if (entry.is_directory()) {
            std::string manifest = entry.path().string() + "/manifest.json";
            if (fs::exists(manifest)) {
                AppMetadata metadata = load_metadata(manifest);
                if (!metadata.id.empty()) {
                    apps.push_back(metadata);
                }
            }
        }
    }
    
    LOG_INFO("AppRegistry", "Scanned ", apps.size(), " apps from: ", apps_dir);
    return apps;
}

} // namespace app
} // namespace touchdown
