#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <curl/curl.h>

namespace Aetherwave {

/**
 * @brief Represents a color in SDL2 format
 */
struct AestheticColor {
    SDL_Color color;
    std::string hex;
    float brightness;
    float saturation;

    AestheticColor(const std::string& hexColor);
    AestheticColor(const SDL_Color& sdlColor);

    SDL_Color toSDL() const { return color; }
    std::string toHex() const { return hex; }
};

/**
 * @brief Stores complete aesthetic theme information for SDL2
 */
struct AestheticTheme {
    std::string name;
    float confidence;

    // Color palette
    std::vector<AestheticColor> primaryColors;
    std::vector<AestheticColor> accentColors;
    AestheticColor backgroundColor;

    // Visual properties
    std::string temperatureBias;  // warm/cool/neutral
    std::string energyLevel;      // low/medium/high
    float complexityPreference;   // 0-1

    // Transition properties
    std::string transitionType;   // fade/glitch/pixel/dissolve
    float transitionDuration;     // seconds
    float effectIntensity;        // 0-1

    AestheticTheme();
    static AestheticTheme getDefaultTheme();
};

/**
 * @brief HTTP response structure for theme API calls
 */
struct ThemeResponse {
    bool success;
    AestheticTheme theme;
    std::string error;
};

/**
 * @brief Manages visual themes and Python API integration for SDL2
 */
class ThemeManagerSDL {
private:
    AestheticTheme currentTheme;
    std::string apiBaseUrl;
    bool apiAvailable;

    // HTTP client for Python API calls
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::string makeHttpRequest(const std::string& url, const std::string& postData = "");

public:
    ThemeManagerSDL(const std::string& apiUrl = "http://localhost:8000");
    ~ThemeManagerSDL();

    // Theme management
    bool loadThemeFromAPI(const std::vector<std::string>& imagePaths);
    AestheticTheme getCurrentTheme() const { return currentTheme; }
    void setTheme(const AestheticTheme& theme) { currentTheme = theme; }

    // API integration
    bool testAPIConnection();
    ThemeResponse analyzeCollectionTheme(const std::vector<std::string>& imagePaths);

    // Utility methods
    SDL_Color getBackgroundColor() const;
    SDL_Color getPrimaryColor() const;
    SDL_Color getAccentColor() const;
    std::string getTransitionType() const;
    float getTransitionDuration() const;
    float getEffectIntensity() const;

    // Debug info
    void printThemeInfo() const;
};

} // namespace Aetherwave
