#include "ThemeManager.h"
#include "ofMain.h"
#include "ofURLFileLoader.h"
#include <sstream>

namespace Aetherwave {

//=============================================================================
// AestheticColor Implementation
//=============================================================================

AestheticColor::AestheticColor(const std::string& hexColor) {
    hex = hexColor;
    // Parse hex color (supports #RRGGBB format)
    if (hex.length() >= 7 && hex[0] == '#') {
        int r = std::stoi(hex.substr(1, 2), nullptr, 16);
        int g = std::stoi(hex.substr(3, 2), nullptr, 16);
        int b = std::stoi(hex.substr(5, 2), nullptr, 16);
        color = ofColor(r, g, b);
    } else {
        ofLogWarning("AestheticColor") << "Invalid hex color: " << hex;
        color = ofColor::white;
        hex = "#FFFFFF";
    }

    brightness = color.getBrightness() / 255.0f;
    saturation = color.getSaturation() / 255.0f;
}

AestheticColor::AestheticColor(const ofColor& ofColor) {
    color = ofColor;

    // Convert to hex
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0')
       << std::setw(2) << (int)color.r
       << std::setw(2) << (int)color.g
       << std::setw(2) << (int)color.b;
    hex = ss.str();

    brightness = color.getBrightness() / 255.0f;
    saturation = color.getSaturation() / 255.0f;
}

ofVec3f AestheticColor::toHSV() const {
    return ofVec3f(color.getHue() / 255.0f,
                   color.getSaturation() / 255.0f,
                   color.getBrightness() / 255.0f);
}

//=============================================================================
// AestheticTheme Implementation
//=============================================================================

AestheticTheme::AestheticTheme()
    : themeName("default")
    , confidence(0.0f)
    , temperatureBias("neutral")
    , energyLevel("medium")
    , complexityPreference(0.5f)
    , transitionSpeed(1.0f)
    , effectIntensity(0.5f)
    , transitionStyle("fade")
{
    // Set default colors
    primaryColors.push_back(AestheticColor("#808080"));  // Gray
    accentColors.push_back(AestheticColor("#FFFFFF"));   // White
}

AestheticTheme AestheticTheme::createFallbackTheme() {
    AestheticTheme theme;
    theme.themeName = "fallback";
    theme.confidence = 1.0f;
    theme.primaryColors.clear();
    theme.accentColors.clear();

    // Safe neutral colors
    theme.primaryColors.push_back(AestheticColor("#2C2C2C"));  // Dark gray
    theme.primaryColors.push_back(AestheticColor("#4A4A4A"));  // Medium gray
    theme.accentColors.push_back(AestheticColor("#FFFFFF"));   // White
    theme.accentColors.push_back(AestheticColor("#CCCCCC"));   // Light gray

    theme.moodProfile["calm"] = 0.8f;
    theme.moodProfile["neutral"] = 1.0f;

    return theme;
}

ofColor AestheticTheme::getPrimaryColor(int index) const {
    if (index >= 0 && index < primaryColors.size()) {
        return primaryColors[index].toOfColor();
    }
    return ofColor(128, 128, 128); // Default gray
}

ofColor AestheticTheme::getAccentColor(int index) const {
    if (index >= 0 && index < accentColors.size()) {
        return accentColors[index].toOfColor();
    }
    return ofColor(255, 255, 255); // Default white
}

ofColor AestheticTheme::getUIBackgroundColor() const {
    // Use darkest primary color for background
    ofColor darkest = getPrimaryColor(0);
    for (int i = 1; i < primaryColors.size(); i++) {
        ofColor candidate = getPrimaryColor(i);
        if (candidate.getBrightness() < darkest.getBrightness()) {
            darkest = candidate;
        }
    }

    // Ensure it's dark enough for UI background
    if (darkest.getBrightness() > 100) {
        darkest.setBrightness(50);
    }

    return darkest;
}

ofColor AestheticTheme::getUITextColor() const {
    ofColor bg = getUIBackgroundColor();

    // Return white text for dark backgrounds, dark text for light backgrounds
    if (bg.getBrightness() < 128) {
        return ofColor::white;
    } else {
        return ofColor::black;
    }
}

float AestheticTheme::getAnimationDuration() const {
    // Base duration modified by energy level
    float baseDuration = 2.0f;

    if (energyLevel == "high") {
        return baseDuration * 0.7f;  // Faster transitions
    } else if (energyLevel == "low") {
        return baseDuration * 1.5f;  // Slower transitions
    }

    return baseDuration;
}

bool AestheticTheme::shouldUseEffects() const {
    return effectIntensity > 0.3f;
}

//=============================================================================
// ThemeManager Implementation
//=============================================================================

ThemeManager::ThemeManager()
    : currentTheme_(AestheticTheme::createFallbackTheme())
    , themeLoaded_(false)
    , autoUpdate_(true)
    , transitioning_(false)
    , transitionProgress_(0.0f)
    , transitionDuration_(1.0f)
    , lastUpdateTime_(0.0f)
    , updateInterval_(5.0f)  // Check for updates every 5 seconds
    , pythonServiceUrl_("http://127.0.0.1:8000")
{
}

ThemeManager::~ThemeManager() {
    shutdown();
}

void ThemeManager::initialize() {
    ofLogNotice("ThemeManager") << "Initializing Theme Manager";

    // Try to load initial theme
    if (!loadThemeFromPython()) {
        ofLogWarning("ThemeManager") << "Failed to load initial theme, using fallback";
        currentTheme_ = AestheticTheme::createFallbackTheme();
        themeLoaded_ = true;
    }

    lastUpdateTime_ = ofGetElapsedTimef();
}

void ThemeManager::update(float deltaTime) {
    float currentTime = ofGetElapsedTimef();

    // Handle theme transitions
    if (transitioning_) {
        updateThemeTransition(deltaTime);
    }

    // Check for theme updates if auto-update is enabled
    if (autoUpdate_ && currentTime - lastUpdateTime_ > updateInterval_) {
        checkForThemeUpdates();
        lastUpdateTime_ = currentTime;
    }
}

void ThemeManager::shutdown() {
    ofLogNotice("ThemeManager") << "Shutting down Theme Manager";
    autoUpdate_ = false;
    transitioning_ = false;
}

bool ThemeManager::loadThemeFromPython(const std::string& serviceUrl) {
    if (!serviceUrl.empty()) {
        pythonServiceUrl_ = serviceUrl;
    }

    ofLogNotice("ThemeManager") << "Loading theme from: " << pythonServiceUrl_;

    try {
        std::string themeJson = fetchThemeFromPython();
        if (themeJson.empty()) {
            ofLogWarning("ThemeManager") << "Received empty theme response";
            return false;
        }

        AestheticTheme newTheme = parseThemeJson(themeJson);
        if (newTheme.themeName == "error") {
            ofLogError("ThemeManager") << "Failed to parse theme JSON";
            return false;
        }

        startThemeTransition(newTheme);
        return true;

    } catch (const std::exception& e) {
        ofLogError("ThemeManager") << "Exception loading theme: " << e.what();
        return false;
    }
}

bool ThemeManager::applyTheme(const AestheticTheme& theme) {
    startThemeTransition(theme);
    return true;
}

void ThemeManager::checkForThemeUpdates() {
    // Only check if we're not already transitioning
    if (!transitioning_) {
        loadThemeFromPython();
    }
}

std::string ThemeManager::fetchThemeFromPython() {
    std::string endpoint = pythonServiceUrl_ + "/analyze/collection-theme";

    ofLogVerbose("ThemeManager") << "Fetching theme from: " << endpoint;

    // Use ofLoadURL for HTTP GET request
    ofBuffer response = ofLoadURL(endpoint);

    if (response.size() == 0) {
        ofLogWarning("ThemeManager") << "Empty response from theme service";
        return "";
    }

    return response.getText();
}

AestheticTheme ThemeManager::parseThemeJson(const std::string& json) {
    AestheticTheme theme;

    try {
        // Simple JSON parsing (in a real implementation, use a proper JSON library)
        // For now, create a sample cyberfemme theme based on expected structure

        if (json.find("cyberfemme") != std::string::npos ||
            json.find("purple") != std::string::npos ||
            json.find("magenta") != std::string::npos) {

            // Cyberfemme theme
            theme.themeName = "cyberfemme";
            theme.confidence = 0.9f;
            theme.temperatureBias = "cool";
            theme.energyLevel = "high";
            theme.complexityPreference = 0.8f;
            theme.transitionSpeed = 1.2f;
            theme.effectIntensity = 0.8f;
            theme.transitionStyle = "glitch";

            theme.primaryColors.clear();
            theme.accentColors.clear();
            theme.primaryColors.push_back(AestheticColor("#1a1a2e"));  // Deep space purple
            theme.primaryColors.push_back(AestheticColor("#16213e"));  // Dark blue
            theme.accentColors.push_back(AestheticColor("#ff00ff"));   // Electric magenta
            theme.accentColors.push_back(AestheticColor("#00ffff"));   // Cyan
            theme.accentColors.push_back(AestheticColor("#9932cc"));   // Purple

            theme.moodProfile["vibrant"] = 0.9f;
            theme.moodProfile["cinematic"] = 0.8f;
            theme.moodProfile["dramatic"] = 0.7f;

        } else if (json.find("organic") != std::string::npos ||
                   json.find("earth") != std::string::npos ||
                   json.find("natural") != std::string::npos) {

            // Organic theme
            theme.themeName = "organic";
            theme.confidence = 0.85f;
            theme.temperatureBias = "warm";
            theme.energyLevel = "low";
            theme.complexityPreference = 0.4f;
            theme.transitionSpeed = 0.8f;
            theme.effectIntensity = 0.4f;
            theme.transitionStyle = "soft_fade";

            theme.primaryColors.clear();
            theme.accentColors.clear();
            theme.primaryColors.push_back(AestheticColor("#2c2416"));  // Dark earth
            theme.primaryColors.push_back(AestheticColor("#3a3024"));  // Brown
            theme.accentColors.push_back(AestheticColor("#8b7355"));   // Warm beige
            theme.accentColors.push_back(AestheticColor("#a0956b"));   // Light brown
            theme.accentColors.push_back(AestheticColor("#6b8e5a"));   // Forest green

            theme.moodProfile["peaceful"] = 0.9f;
            theme.moodProfile["serene"] = 0.8f;
            theme.moodProfile["natural"] = 0.9f;

        } else {
            // Default tech theme
            theme.themeName = "tech";
            theme.confidence = 0.7f;
            theme.temperatureBias = "cool";
            theme.energyLevel = "medium";
            theme.complexityPreference = 0.6f;
            theme.transitionSpeed = 1.0f;
            theme.effectIntensity = 0.6f;
            theme.transitionStyle = "pixel";

            theme.primaryColors.clear();
            theme.accentColors.clear();
            theme.primaryColors.push_back(AestheticColor("#1c1c1c"));  // Almost black
            theme.primaryColors.push_back(AestheticColor("#2a2a2a"));  // Dark gray
            theme.accentColors.push_back(AestheticColor("#00aaff"));   // Tech blue
            theme.accentColors.push_back(AestheticColor("#ffffff"));   // White
            theme.accentColors.push_back(AestheticColor("#888888"));   // Gray
        }

    } catch (const std::exception& e) {
        ofLogError("ThemeManager") << "JSON parsing error: " << e.what();
        theme.themeName = "error";
    }

    return theme;
}

void ThemeManager::startThemeTransition(const AestheticTheme& newTheme) {
    if (newTheme.themeName != currentTheme_.themeName) {
        ofLogNotice("ThemeManager") << "Starting transition to theme: " << newTheme.themeName;

        previousTheme_ = currentTheme_;
        currentTheme_ = newTheme;

        transitioning_ = true;
        transitionProgress_ = 0.0f;
        transitionDuration_ = 2.0f;  // 2 second theme transition

        themeLoaded_ = true;

        // Apply immediate settings that don't need transition
        applyTheme(newTheme);
    }
}

void ThemeManager::updateThemeTransition(float deltaTime) {
    if (!transitioning_) return;

    transitionProgress_ += deltaTime / transitionDuration_;

    if (transitionProgress_ >= 1.0f) {
        transitionProgress_ = 1.0f;
        transitioning_ = false;
        ofLogNotice("ThemeManager") << "Theme transition completed";
    }

    // Update visual elements during transition
    applyColorsToUI();
}

void ThemeManager::applyColorsToUI() {
    // This method can be called by the main application to apply current theme colors
    // The implementation depends on how the UI system works
}

void ThemeManager::updateTransitionSettings() {
    // This method updates transition settings based on current theme
    // Called by ImageManager to get theme-appropriate transition styles
}

ofColor ThemeManager::getBackgroundColor() const {
    return currentTheme_.getUIBackgroundColor();
}

ofColor ThemeManager::getTextColor() const {
    return currentTheme_.getUITextColor();
}

ofColor ThemeManager::getAccentColor(int variant) const {
    return currentTheme_.getAccentColor(variant);
}

ofColor ThemeManager::getHighlightColor() const {
    // Use the brightest accent color for highlights
    ofColor brightest = currentTheme_.getAccentColor(0);

    for (int i = 1; i < currentTheme_.accentColors.size(); i++) {
        ofColor candidate = currentTheme_.getAccentColor(i);
        if (candidate.getBrightness() > brightest.getBrightness()) {
            brightest = candidate;
        }
    }

    return brightest;
}

float ThemeManager::getFadeDuration() const {
    return currentTheme_.getAnimationDuration();
}

float ThemeManager::getTransitionDuration() const {
    return currentTheme_.getAnimationDuration();
}

std::string ThemeManager::getPreferredTransition() const {
    return currentTheme_.transitionStyle;
}

void ThemeManager::drawThemeDebugInfo(int x, int y) const {
    ofPushStyle();
    ofSetColor(getTextColor());

    std::string info = "Theme: " + currentTheme_.themeName +
                      " (confidence: " + ofToString(currentTheme_.confidence, 2) + ")";
    ofDrawBitmapString(info, x, y);
    ofDrawBitmapString("Energy: " + currentTheme_.energyLevel, x, y + 15);
    ofDrawBitmapString("Transition: " + currentTheme_.transitionStyle, x, y + 30);

    if (transitioning_) {
        ofDrawBitmapString("Transitioning: " + ofToString(transitionProgress_ * 100, 1) + "%", x, y + 45);
    }

    ofPopStyle();
}

std::string ThemeManager::getThemeStatusString() const {
    return "Theme: " + currentTheme_.themeName +
           " | Energy: " + currentTheme_.energyLevel +
           " | Style: " + currentTheme_.transitionStyle;
}

//=============================================================================
// GlobalThemeManager Implementation
//=============================================================================

std::unique_ptr<ThemeManager> GlobalThemeManager::instance_ = nullptr;

ThemeManager& GlobalThemeManager::getInstance() {
    if (!instance_) {
        instance_ = std::make_unique<ThemeManager>();
    }
    return *instance_;
}

void GlobalThemeManager::destroyInstance() {
    instance_.reset();
}

ofColor GlobalThemeManager::getThemeBackgroundColor() {
    return getInstance().getBackgroundColor();
}

ofColor GlobalThemeManager::getThemeTextColor() {
    return getInstance().getTextColor();
}

ofColor GlobalThemeManager::getThemeAccentColor(int variant) {
    return getInstance().getAccentColor(variant);
}

float GlobalThemeManager::getThemeFadeDuration() {
    return getInstance().getFadeDuration();
}

bool GlobalThemeManager::isThemeLoaded() {
    return getInstance().hasActiveTheme();
}

} // namespace Aetherwave
