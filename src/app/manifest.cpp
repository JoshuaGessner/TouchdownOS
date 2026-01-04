/**
 * @file manifest.cpp
 * @brief Manifest parser implementation
 */

#include "touchdown/app/manifest.hpp"
#include "touchdown/core/logger.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace touchdown {
namespace app {

std::optional<AppMetadata> ManifestParser::parse_file(const std::string& manifest_path) {
    std::ifstream file(manifest_path);
    if (!file.is_open()) {
        LOG_ERROR("ManifestParser", "Failed to open manifest: ", manifest_path);
        return std::nullopt;
    }
    
    try {
        json j;
        file >> j;
        
        AppMetadata metadata;
        metadata.id = j.value("id", "");
        metadata.name = j.value("name", "");
        metadata.version = j.value("version", "");
        metadata.description = j.value("description", "");
        metadata.icon = j.value("icon", "");
        
        // Parse color
        std::string color_str = j.value("color", "#FFFFFF");
        metadata.color = parse_color(color_str);
        
        // Parse permissions
        if (j.contains("permissions") && j["permissions"].is_array()) {
            metadata.permissions = j["permissions"].get<std::vector<std::string>>();
        }
        
        // Validate
        if (!validate(metadata)) {
            LOG_ERROR("ManifestParser", "Invalid manifest: ", manifest_path);
            return std::nullopt;
        }
        
        LOG_INFO("ManifestParser", "Loaded manifest: ", metadata.id);
        return metadata;
        
    } catch (const json::exception& e) {
        LOG_ERROR("ManifestParser", "JSON parse error: ", e.what());
        return std::nullopt;
    }
}

std::optional<AppMetadata> ManifestParser::parse_string(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        
        AppMetadata metadata;
        metadata.id = j.value("id", "");
        metadata.name = j.value("name", "");
        metadata.version = j.value("version", "");
        metadata.description = j.value("description", "");
        metadata.icon = j.value("icon", "");
        
        std::string color_str = j.value("color", "#FFFFFF");
        metadata.color = parse_color(color_str);
        
        if (j.contains("permissions") && j["permissions"].is_array()) {
            metadata.permissions = j["permissions"].get<std::vector<std::string>>();
        }
        
        if (!validate(metadata)) {
            return std::nullopt;
        }
        
        return metadata;
        
    } catch (const json::exception& e) {
        LOG_ERROR("ManifestParser", "JSON parse error: ", e.what());
        return std::nullopt;
    }
}

bool ManifestParser::validate(const AppMetadata& metadata) {
    auto result = ManifestValidator::validate(metadata);
    
    if (!result.valid) {
        for (const auto& error : result.errors) {
            LOG_ERROR("ManifestParser", "Validation error: ", error);
        }
        return false;
    }
    
    for (const auto& warning : result.warnings) {
        LOG_WARNING("ManifestParser", "Validation warning: ", warning);
    }
    
    return true;
}

bool ManifestParser::is_supported_type(const std::string& type) {
    return type == "cpp" || type == "python";
}

std::string ManifestParser::get_app_type(const AppMetadata& metadata) {
    // Type is stored in description for now
    // In full implementation, add 'type' field to AppMetadata
    return "cpp";  // Default
}

std::string ManifestParser::get_main_entry(const AppMetadata& metadata) {
    // Main entry is stored in description for now
    // In full implementation, add 'main' field to AppMetadata
    return "";
}

lv_color_t ManifestParser::parse_color(const std::string& color_str) {
    if (color_str.empty() || color_str[0] != '#') {
        return lv_color_white();
    }
    
    try {
        std::string hex = color_str.substr(1);
        uint32_t color_val = std::stoul(hex, nullptr, 16);
        return lv_color_hex(color_val);
    } catch (...) {
        return lv_color_white();
    }
}

// Validator implementation

ManifestValidator::ValidationResult ManifestValidator::validate(const AppMetadata& metadata) {
    ValidationResult result;
    result.valid = true;
    
    // Validate ID
    if (!validate_id(metadata.id, result.errors)) {
        result.valid = false;
    }
    
    // Validate name
    if (metadata.name.empty()) {
        result.errors.push_back("App name is required");
        result.valid = false;
    }
    
    // Validate version
    if (!validate_version(metadata.version, result.errors)) {
        result.valid = false;
    }
    
    // Validate permissions
    if (!validate_permissions(metadata.permissions, result.errors)) {
        result.valid = false;
    }
    
    // Warnings
    if (metadata.description.empty()) {
        result.warnings.push_back("App description is empty");
    }
    
    if (metadata.icon.empty()) {
        result.warnings.push_back("App icon is not set");
    }
    
    return result;
}

bool ManifestValidator::validate_id(const std::string& id, std::vector<std::string>& errors) {
    if (id.empty()) {
        errors.push_back("App ID is required");
        return false;
    }
    
    // Check ID format (alphanumeric, dots, underscores)
    std::regex id_regex("^[a-zA-Z0-9._-]+$");
    if (!std::regex_match(id, id_regex)) {
        errors.push_back("App ID contains invalid characters");
        return false;
    }
    
    return true;
}

bool ManifestValidator::validate_version(const std::string& version, 
                                        std::vector<std::string>& errors) {
    if (version.empty()) {
        errors.push_back("Version is required");
        return false;
    }
    
    // Basic semantic version check
    std::regex version_regex("^\\d+\\.\\d+\\.\\d+$");
    if (!std::regex_match(version, version_regex)) {
        errors.push_back("Version must be in format X.Y.Z");
        return false;
    }
    
    return true;
}

bool ManifestValidator::validate_permissions(const std::vector<std::string>& permissions,
                                            std::vector<std::string>& errors) {
    // Known permissions
    static const std::vector<std::string> valid_permissions = {
        "system.settings",
        "system.stats",
        "power.control",
        "network.access",
        "bluetooth.access",
        "storage.read",
        "storage.write"
    };
    
    for (const auto& perm : permissions) {
        auto it = std::find(valid_permissions.begin(), valid_permissions.end(), perm);
        if (it == valid_permissions.end()) {
            errors.push_back("Unknown permission: " + perm);
            return false;
        }
    }
    
    return true;
}

} // namespace app
} // namespace touchdown
