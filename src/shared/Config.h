#pragma once

#include <string>
#include <vector>

namespace Aetherwave {
namespace Shared {

/**
 * Configuration utility for loading and managing application settings.
 * Provides shared functionality between C++ and Python components.
 */
class ConfigManager {
public:
    // Configuration file paths
    static constexpr const char* CONFIG_FILE = "config/config.yaml";
    static constexpr const char* SCHEMA_FILE = "config/schema.yaml";

    // Default values
    static constexpr int DEFAULT_WIDTH = 1920;
    static constexpr int DEFAULT_HEIGHT = 1080;
    static constexpr int DEFAULT_FPS = 60;
    static constexpr float DEFAULT_FADE_DURATION = 2.0f;

    /**
     * Load configuration from YAML file.
     * Future implementation will parse actual YAML.
     */
    static bool loadConfig(const std::string& configPath = CONFIG_FILE);

    /**
     * Get supported image file extensions.
     */
    static std::vector<std::string> getSupportedFormats();

    /**
     * Validate file extension against supported formats.
     */
    static bool isValidImageFile(const std::string& filename);

private:
    static std::vector<std::string> supportedFormats;
};

/**
 * Utility functions for file path manipulation and validation.
 */
namespace FileUtils {
    /**
     * Extract file extension from path.
     */
    std::string getExtension(const std::string& filepath);

    /**
     * Get filename without extension.
     */
    std::string getBasename(const std::string& filepath);

    /**
     * Check if file exists (stub implementation).
     */
    bool fileExists(const std::string& filepath);
}

} // namespace Shared
} // namespace Aetherwave
