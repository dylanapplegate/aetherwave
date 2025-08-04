#include "ThemeManagerSDL.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Aetherwave {

// Color utility functions
SDL_Color hexToSDL(const std::string& hex) {
    SDL_Color color = {0, 0, 0, 255};
    if (hex.length() >= 7 && hex[0] == '#') {
        try {
            unsigned long value = std::stoul(hex.substr(1), nullptr, 16);
            color.r = (value >> 16) & 0xFF;
            color.g = (value >> 8) & 0xFF;
            color.b = value & 0xFF;
        } catch (...) {
            std::cerr << "⚠️ Invalid hex color: " << hex << std::endl;
        }
    }
    return color;
}

std::string sdlToHex(const SDL_Color& color) {
    std::ostringstream oss;
    oss << "#" << std::hex << std::setfill('0') 
        << std::setw(2) << (int)color.r
        << std::setw(2) << (int)color.g  
        << std::setw(2) << (int)color.b;
    return oss.str();
}

// AestheticColor implementation
AestheticColor::AestheticColor(const std::string& hexColor) : hex(hexColor) {
    color = hexToSDL(hexColor);
    brightness = (color.r + color.g + color.b) / 765.0f;
    float max = std::max({color.r, color.g, color.b}) / 255.0f;
    float min = std::min({color.r, color.g, color.b}) / 255.0f;
    saturation = max > 0 ? (max - min) / max : 0;
}

AestheticColor::AestheticColor(const SDL_Color& sdlColor) : color(sdlColor) {
    hex = sdlToHex(sdlColor);
    brightness = (color.r + color.g + color.b) / 765.0f;
    float max = std::max({color.r, color.g, color.b}) / 255.0f;
    float min = std::min({color.r, color.g, color.b}) / 255.0f;
    saturation = max > 0 ? (max - min) / max : 0;
}

// AestheticTheme implementation
AestheticTheme::AestheticTheme() 
    : name("default")
    , confidence(1.0f)
    , backgroundColor(AestheticColor("#1a1a2e"))
    , temperatureBias("neutral")
    , energyLevel("medium")
    , complexityPreference(0.5f)
    , transitionType("fade")
    , transitionDuration(1.0f)
    , effectIntensity(0.5f) {
    
    primaryColors.push_back(AestheticColor("#2d2d4d"));
    accentColors.push_back(AestheticColor("#ffffff"));
}

AestheticTheme AestheticTheme::getDefaultTheme() {
    return AestheticTheme();
}

// ThemeManagerSDL implementation
ThemeManagerSDL::ThemeManagerSDL(const std::string& apiUrl) 
    : apiBaseUrl(apiUrl)
    , apiAvailable(false)
    , currentTheme(AestheticTheme::getDefaultTheme()) {
    
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test API connection
    apiAvailable = testAPIConnection();
    
    if (apiAvailable) {
        std::cout << "✅ Python API connected: " << apiBaseUrl << std::endl;
    } else {
        std::cout << "⚠️ Python API unavailable, using default theme" << std::endl;
    }
}

ThemeManagerSDL::~ThemeManagerSDL() {
    curl_global_cleanup();
}

size_t ThemeManagerSDL::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string ThemeManagerSDL::makeHttpRequest(const std::string& url, const std::string& postData) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        if (!postData.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
            
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "❌ HTTP request failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    
    return readBuffer;
}

bool ThemeManagerSDL::testAPIConnection() {
    std::string response = makeHttpRequest(apiBaseUrl + "/health");
    return !response.empty() && response.find("healthy") != std::string::npos;
}

bool ThemeManagerSDL::loadThemeFromAPI(const std::vector<std::string>& imagePaths) {
    if (!apiAvailable) {
        std::cout << "⚠️ API unavailable, keeping current theme" << std::endl;
        return false;
    }
    
    ThemeResponse result = analyzeCollectionTheme(imagePaths);
    if (result.success) {
        currentTheme = result.theme;
        std::cout << "🎨 Loaded theme: " << currentTheme.name 
                  << " (confidence: " << currentTheme.confidence << ")" << std::endl;
        return true;
    } else {
        std::cerr << "❌ Theme loading failed: " << result.error << std::endl;
        return false;
    }
}

ThemeResponse ThemeManagerSDL::analyzeCollectionTheme(const std::vector<std::string>& imagePaths) {
    ThemeResponse response;
    response.success = false;
    
    if (!apiAvailable) {
        response.error = "API not available";
        return response;
    }
    
    // Build JSON request
    std::ostringstream json;
    json << "{\"images\":[";
    for (size_t i = 0; i < imagePaths.size(); ++i) {
        json << "\"" << imagePaths[i] << "\"";
        if (i < imagePaths.size() - 1) json << ",";
    }
    json << "]}";
    
    std::string apiResponse = makeHttpRequest(apiBaseUrl + "/analyze/collection-theme", json.str());
    
    if (apiResponse.empty()) {
        response.error = "Empty API response";
        return response;
    }
    
    // Parse JSON response (simplified - in production would use proper JSON parser)
    if (apiResponse.find("\"ok\":true") != std::string::npos) {
        response.success = true;
        
        // Extract theme name
        size_t namePos = apiResponse.find("\"theme_name\":\"");
        if (namePos != std::string::npos) {
            namePos += 14;
            size_t nameEnd = apiResponse.find("\"", namePos);
            if (nameEnd != std::string::npos) {
                response.theme.name = apiResponse.substr(namePos, nameEnd - namePos);
            }
        }
        
        // Extract confidence
        size_t confPos = apiResponse.find("\"confidence\":");
        if (confPos != std::string::npos) {
            confPos += 13;
            size_t confEnd = apiResponse.find(",", confPos);
            if (confEnd == std::string::npos) confEnd = apiResponse.find("}", confPos);
            if (confEnd != std::string::npos) {
                try {
                    response.theme.confidence = std::stof(apiResponse.substr(confPos, confEnd - confPos));
                } catch (...) {
                    response.theme.confidence = 0.5f;
                }
            }
        }
        
        // Set theme-specific properties
        if (response.theme.name == "cyberfemme") {
            response.theme.primaryColors.clear();
            response.theme.primaryColors.push_back(AestheticColor("#ff00ff"));
            response.theme.primaryColors.push_back(AestheticColor("#9932cc"));
            response.theme.accentColors.clear();
            response.theme.accentColors.push_back(AestheticColor("#00ffff"));
            response.theme.backgroundColor = AestheticColor("#1a1a2e");
            response.theme.transitionType = "glitch";
            response.theme.effectIntensity = 0.8f;
            response.theme.energyLevel = "high";
        } else if (response.theme.name == "organic") {
            response.theme.primaryColors.clear();
            response.theme.primaryColors.push_back(AestheticColor("#8fbc8f"));
            response.theme.primaryColors.push_back(AestheticColor("#deb887"));
            response.theme.accentColors.clear();
            response.theme.accentColors.push_back(AestheticColor("#f5f5dc"));
            response.theme.backgroundColor = AestheticColor("#2f4f2f");
            response.theme.transitionType = "soft_fade";
            response.theme.effectIntensity = 0.3f;
            response.theme.energyLevel = "low";
        } else {
            // Default adaptive theme
            response.theme.primaryColors.clear();
            response.theme.primaryColors.push_back(AestheticColor("#2d2d4d"));
            response.theme.accentColors.clear();
            response.theme.accentColors.push_back(AestheticColor("#ffffff"));
            response.theme.backgroundColor = AestheticColor("#1a1a2e");
            response.theme.transitionType = "fade";
            response.theme.effectIntensity = 0.5f;
            response.theme.energyLevel = "medium";
        }
        
    } else {
        response.error = "API returned error";
    }
    
    return response;
}

SDL_Color ThemeManagerSDL::getBackgroundColor() const {
    return currentTheme.backgroundColor.toSDL();
}

SDL_Color ThemeManagerSDL::getPrimaryColor() const {
    if (!currentTheme.primaryColors.empty()) {
        return currentTheme.primaryColors[0].toSDL();
    }
    return {45, 45, 77, 255}; // Default
}

SDL_Color ThemeManagerSDL::getAccentColor() const {
    if (!currentTheme.accentColors.empty()) {
        return currentTheme.accentColors[0].toSDL();
    }
    return {255, 255, 255, 255}; // Default white
}

std::string ThemeManagerSDL::getTransitionType() const {
    return currentTheme.transitionType;
}

float ThemeManagerSDL::getTransitionDuration() const {
    return currentTheme.transitionDuration;
}

float ThemeManagerSDL::getEffectIntensity() const {
    return currentTheme.effectIntensity;
}

void ThemeManagerSDL::printThemeInfo() const {
    std::cout << "🎨 Current Theme: " << currentTheme.name << std::endl;
    std::cout << "   Confidence: " << currentTheme.confidence << std::endl;
    std::cout << "   Transition: " << currentTheme.transitionType << std::endl;
    std::cout << "   Energy: " << currentTheme.energyLevel << std::endl;
    std::cout << "   Background: " << currentTheme.backgroundColor.toHex() << std::endl;
    if (!currentTheme.primaryColors.empty()) {
        std::cout << "   Primary: " << currentTheme.primaryColors[0].toHex() << std::endl;
    }
}

} // namespace Aetherwave
