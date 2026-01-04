/**
 * @file app_registry.hpp
 * @brief Registry for managing installed apps
 */

#ifndef TOUCHDOWN_APP_APP_REGISTRY_HPP
#define TOUCHDOWN_APP_APP_REGISTRY_HPP

#include "touchdown/app/app.hpp"
#include <map>
#include <vector>

namespace touchdown {
namespace app {

class AppRegistry {
public:
    static AppRegistry& instance();
    
    /**
     * @brief Register an app factory
     */
    bool register_app(const std::string& app_id, AppFactory factory);
    
    /**
     * @brief Create an app instance
     */
    std::unique_ptr<TouchdownApp> create_app(const std::string& app_id, 
                                             const AppMetadata& metadata);
    
    /**
     * @brief Get list of registered app IDs
     */
    std::vector<std::string> get_app_ids() const;
    
    /**
     * @brief Check if app is registered
     */
    bool has_app(const std::string& app_id) const;
    
    /**
     * @brief Load app metadata from manifest file
     */
    AppMetadata load_metadata(const std::string& manifest_path);
    
    /**
     * @brief Scan directory for app manifests
     */
    std::vector<AppMetadata> scan_apps(const std::string& apps_dir);
    
private:
    AppRegistry() = default;
    
    std::map<std::string, AppFactory> factories_;
};

} // namespace app
} // namespace touchdown

#endif // TOUCHDOWN_APP_APP_REGISTRY_HPP
