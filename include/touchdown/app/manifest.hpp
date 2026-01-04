/**
 * @file manifest.hpp
 * @brief App manifest parser and validator
 */

#ifndef TOUCHDOWN_APP_MANIFEST_HPP
#define TOUCHDOWN_APP_MANIFEST_HPP

#include "touchdown/app/app.hpp"
#include <string>
#include <optional>

namespace touchdown {
namespace app {

/**
 * @brief App manifest parser
 */
class ManifestParser {
public:
    /**
     * @brief Parse manifest from JSON file
     * @param manifest_path Path to manifest.json
     * @return AppMetadata if valid, empty if invalid
     */
    static std::optional<AppMetadata> parse_file(const std::string& manifest_path);
    
    /**
     * @brief Parse manifest from JSON string
     */
    static std::optional<AppMetadata> parse_string(const std::string& json);
    
    /**
     * @brief Validate manifest structure
     */
    static bool validate(const AppMetadata& metadata);
    
    /**
     * @brief Check if app type is supported
     */
    static bool is_supported_type(const std::string& type);
    
    /**
     * @brief Get app type from metadata
     */
    static std::string get_app_type(const AppMetadata& metadata);
    
    /**
     * @brief Get main entry point from metadata
     */
    static std::string get_main_entry(const AppMetadata& metadata);
    
private:
    static lv_color_t parse_color(const std::string& color_str);
};

/**
 * @brief App manifest validator
 */
class ManifestValidator {
public:
    struct ValidationResult {
        bool valid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    /**
     * @brief Validate app manifest
     */
    static ValidationResult validate(const AppMetadata& metadata);
    
private:
    static bool validate_id(const std::string& id, std::vector<std::string>& errors);
    static bool validate_version(const std::string& version, std::vector<std::string>& errors);
    static bool validate_permissions(const std::vector<std::string>& permissions, 
                                    std::vector<std::string>& errors);
};

} // namespace app
} // namespace touchdown

#endif // TOUCHDOWN_APP_MANIFEST_HPP
