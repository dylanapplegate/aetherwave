#include "Config.h"
#include <algorithm>
#include <fstream>

namespace Aetherwave {
namespace Shared {

// Initialize static member
std::vector<std::string> ConfigManager::supportedFormats = {
    ".jpg", ".jpeg", ".png", ".tiff", ".webp"
};

bool ConfigManager::loadConfig(const std::string& configPath) {
    // Stub implementation - future will parse YAML
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // Config file doesn't exist, use defaults
        return false;
    }

    // Future: Parse YAML configuration
    // For now, just return success if file exists
    return true;
}

std::vector<std::string> ConfigManager::getSupportedFormats() {
    return supportedFormats;
}

bool ConfigManager::isValidImageFile(const std::string& filename) {
    std::string ext = FileUtils::getExtension(filename);

    // Convert to lowercase for comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto formats = getSupportedFormats();
    return std::find(formats.begin(), formats.end(), ext) != formats.end();
}

namespace FileUtils {

std::string getExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    return filepath.substr(dotPos);
}

std::string getBasename(const std::string& filepath) {
    size_t slashPos = filepath.find_last_of("/\\");
    size_t dotPos = filepath.find_last_of('.');

    size_t start = (slashPos == std::string::npos) ? 0 : slashPos + 1;
    size_t length = (dotPos == std::string::npos) ?
        std::string::npos : dotPos - start;

    return filepath.substr(start, length);
}

bool fileExists(const std::string& filepath) {
    // Stub implementation
    std::ifstream file(filepath);
    return file.good();
}

} // namespace FileUtils

} // namespace Shared
} // namespace Aetherwave
