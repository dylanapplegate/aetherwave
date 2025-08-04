#pragma once

#include "ofMain.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Aetherwave {

/**
 * @brief Represents a color in various formats for easy conversion
 */
struct AestheticColor {
    ofColor color;
    std::string hex;
    float brightness;
    float saturation;

    AestheticColor(const std::string& hexColor);
    AestheticColor(const ofColor& ofColor);

    // Convert to different formats
    ofColor toOfColor() const { return color; }
    std::string toHex() const { return hex; }
    ofVec3f toHSV() const;
};

/**
 * @brief Stores complete aesthetic theme information
 */
struct AestheticTheme {
    std::string themeName;
    float confidence;

    // Color palettes
    std::vector<AestheticColor> primaryColors;
    std::vector<AestheticColor> accentColors;

    // Theme characteristics
    std::string temperatureBias;  // warm/cool/neutral
    std::string energyLevel;      // low/medium/high
    float complexityPreference;   // 0.0-1.0

    // Mood profile (mood -> weight)
    std::map<std::string, float> moodProfile;

    // Visual effect settings
    float transitionSpeed;        // Based on energy level
    float effectIntensity;        // Based on mood and complexity
    std::string transitionStyle;  // fade/slide/zoom/etc

    AestheticTheme();

    // Factory methods
    static AestheticTheme createFallbackTheme();
    static AestheticTheme fromJsonString(const std::string& json);

    // Theme application helpers
    ofColor getPrimaryColor(int index = 0) const;
    ofColor getAccentColor(int index = 0) const;
    ofColor getUIBackgroundColor() const;
    ofColor getUITextColor() const;
    float getAnimationDuration() const;
    bool shouldUseEffects() const;
};

/**
 * @brief Manages aesthetic themes and applies them to the display system
 */
class ThemeManager {
public:
    ThemeManager();
    ~ThemeManager();

    // Theme lifecycle
    void initialize();
    void update(float deltaTime);
    void shutdown();

    // Theme loading and application
    bool loadThemeFromPython(const std::string& serviceUrl = "http://127.0.0.1:8000");
    bool applyTheme(const AestheticTheme& theme);
    void applyAdaptiveTheme();

    // Theme access
    const AestheticTheme& getCurrentTheme() const { return currentTheme_; }
    bool hasActiveTheme() const { return themeLoaded_; }
    float getThemeConfidence() const { return currentTheme_.confidence; }

    // Visual theme application
    void applyColorsToUI();
    void updateTransitionSettings();
    void updateEffectSettings();

    // Theme-aware color methods
    ofColor getBackgroundColor() const;
    ofColor getTextColor() const;
    ofColor getAccentColor(int variant = 0) const;
    ofColor getHighlightColor() const;

    // Theme-aware animation settings
    float getFadeDuration() const;
    float getTransitionDuration() const;
    std::string getPreferredTransition() const;

    // Adaptive updates
    void checkForThemeUpdates();
    void enableAutoThemeUpdates(bool enable) { autoUpdate_ = enable; }

    // Debug and monitoring
    void drawThemeDebugInfo(int x, int y) const;
    std::string getThemeStatusString() const;

private:
    // Core theme data
    AestheticTheme currentTheme_;
    AestheticTheme previousTheme_;
    bool themeLoaded_;
    bool autoUpdate_;

    // Theme transition state
    bool transitioning_;
    float transitionProgress_;
    float transitionDuration_;

    // Update timing
    float lastUpdateTime_;
    float updateInterval_;  // How often to check for theme changes

    // Python service integration
    std::string pythonServiceUrl_;

    // Internal methods
    std::string fetchThemeFromPython();
    AestheticTheme parseThemeJson(const std::string& json);
    void startThemeTransition(const AestheticTheme& newTheme);
    void updateThemeTransition(float deltaTime);
    AestheticColor interpolateColor(const AestheticColor& from, const AestheticColor& to, float t);

    // Theme application helpers
    void applyThemeToBackgroundRenderer();
    void applyThemeToImageRenderer();
    void applyThemeToTransitionSystem();
    void applyThemeToEffectSystem();

    // Color computation
    ofColor computeUIBackgroundColor() const;
    ofColor computeUITextColor() const;
    ofColor computeHighlightColor() const;

    // Animation computation
    float computeAnimationSpeed() const;
    std::string selectTransitionStyle() const;
    float computeEffectIntensity() const;
};

/**
 * @brief Singleton manager for global theme access
 */
class GlobalThemeManager {
public:
    static ThemeManager& getInstance();
    static void destroyInstance();

    // Convenience methods for global access
    static ofColor getThemeBackgroundColor();
    static ofColor getThemeTextColor();
    static ofColor getThemeAccentColor(int variant = 0);
    static float getThemeFadeDuration();
    static bool isThemeLoaded();

private:
    static std::unique_ptr<ThemeManager> instance_;
    GlobalThemeManager() = default;
};

} // namespace Aetherwave
