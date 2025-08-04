#include "ofMain.h"
#include "DisplayEngine.h"
#include "ThemeManager.h"
#include <memory>

namespace Aetherwave {

/**
 * Enhanced Aetherwave application with production-ready features.
 * Integrates advanced display engine, monitor management, and performance monitoring.
 */
class AetherwaveApp : public ofBaseApp {

public:
    // Core application lifecycle
    void setup() override;
    void update() override;
    void draw() override;

    // Event handlers
    void keyPressed(int key) override;
    void keyReleased(int key) override;
    void windowResized(int w, int h) override;

    // New event handlers
    void mouseMoved(int x, int y) override;
    void mousePressed(int x, int y, int button) override;

private:
    // Core systems
    std::unique_ptr<ImageManager> imageManager;
    std::unique_ptr<MonitorManager> monitorManager;
    std::unique_ptr<PerformanceMonitor> performanceMonitor;
    std::unique_ptr<ThemeManager> themeManager;

    // Application state
    bool isFullscreen;
    bool showPerformanceOverlay;
    bool showThemeDebug;
    bool showWelcomeScreen;
    float welcomeScreenTimer;
    int currentImageIndex;
    float autoAdvanceTimer;
    float autoAdvanceInterval;

    // UI state
    bool showUI;
    float uiOpacity;
    float lastInteractionTime;

    // Constants
    static constexpr float WELCOME_DURATION = 3.0f;
    static constexpr float AUTO_ADVANCE_INTERVAL = 10.0f;
    static constexpr float UI_FADE_DELAY = 3.0f;
    static constexpr float UI_FADE_DURATION = 1.0f;
    static constexpr int TARGET_FPS = 60;

    // Helper methods
    void loadConfiguration();
    void initializeSystems();
    void updateAutoAdvance();
    void updateUI();
    void drawWelcomeScreen();
    void drawImageDisplay();
    void drawUI();
    void drawHelpOverlay();
    void nextImage();
    void previousImage();
    void toggleFullscreen();
    void resetInteractionTimer();
};

void AetherwaveApp::setup() {
    ofLogNotice("Aetherwave") << "=== Aetherwave Display Engine v2.0 ===";

    // Initialize OpenFrameworks settings
    ofSetFrameRate(TARGET_FPS);
    ofSetVerticalSync(true);
    ofBackground(0, 0, 0);
    ofSetLogLevel(OF_LOG_NOTICE);

    // Initialize application state
    isFullscreen = true;
    showPerformanceOverlay = false;
    showThemeDebug = false;
    showWelcomeScreen = true;
    welcomeScreenTimer = 0.0f;
    currentImageIndex = 0;
    autoAdvanceTimer = 0.0f;
    autoAdvanceInterval = AUTO_ADVANCE_INTERVAL;
    showUI = false;
    uiOpacity = 0.0f;
    lastInteractionTime = ofGetElapsedTimef();

    // Load configuration
    loadConfiguration();

    // Initialize core systems
    initializeSystems();

    // Set fullscreen for cinematic experience
    ofSetFullscreen(isFullscreen);

    ofLogNotice("Aetherwave") << "Application initialized successfully";
    ofLogNotice("Aetherwave") << "Fullscreen: " << (isFullscreen ? "ON" : "OFF");
    ofLogNotice("Aetherwave") << "Target FPS: " << TARGET_FPS;
}

void AetherwaveApp::loadConfiguration() {
    // TODO: Load from YAML configuration file
    // For now, use defaults
    ofLogNotice("Aetherwave") << "Loading configuration...";

    // Example configuration loading
    autoAdvanceInterval = AUTO_ADVANCE_INTERVAL;

    ofLogNotice("Aetherwave") << "Auto-advance interval: " << autoAdvanceInterval << "s";
}

void AetherwaveApp::initializeSystems() {
    // Initialize display systems
    imageManager = std::make_unique<ImageManager>();
    monitorManager = std::make_unique<MonitorManager>();
    performanceMonitor = std::make_unique<PerformanceMonitor>();
    themeManager = std::make_unique<ThemeManager>();

    // Initialize theme manager first
    themeManager->initialize();

    // Connect theme manager to image manager
    imageManager->setThemeManager(themeManager.get());

    // Configure image manager
    imageManager->setTransitionDuration(2.0f);
    imageManager->setTransitionType("fade");
    imageManager->setDisplayMode("fit");

    // Try to load images from assets directory
    bool imagesLoaded = imageManager->loadImagesFromDirectory("assets/images");
    if (!imagesLoaded) {
        ofLogWarning("Aetherwave") << "No images found in assets/images directory";
        // Try alternative locations
        imagesLoaded = imageManager->loadImagesFromDirectory("../assets/images");
        if (!imagesLoaded) {
            ofLogWarning("Aetherwave") << "No images found. Please add images to assets/images/";
        }
    }

    if (imagesLoaded) {
        ofLogNotice("Aetherwave") << "Loaded " << imageManager->getImageCount() << " images";
    }

    ofLogNotice("Aetherwave") << "Core systems initialized";
}

void AetherwaveApp::update() {
    float deltaTime = ofGetLastFrameTime();

    // Update performance monitoring
    performanceMonitor->update();

    // Update theme manager
    if (themeManager) {
        themeManager->update(deltaTime);
    }

    // Update welcome screen timer
    if (showWelcomeScreen) {
        welcomeScreenTimer += deltaTime;
        if (welcomeScreenTimer >= WELCOME_DURATION) {
            showWelcomeScreen = false;
            ofLogNotice("Aetherwave") << "Welcome screen completed";
        }
        return; // Skip other updates during welcome screen
    }

    // Update image manager
    if (imageManager) {
        imageManager->update(deltaTime);
    }

    // Update auto-advance timer
    updateAutoAdvance();

    // Update UI
    updateUI();
}

void AetherwaveApp::updateAutoAdvance() {
    if (!imageManager || !imageManager->hasImages() || imageManager->isTransitioning()) {
        return;
    }

    float deltaTime = ofGetLastFrameTime();
    autoAdvanceTimer += deltaTime;

    if (autoAdvanceTimer >= autoAdvanceInterval) {
        nextImage();
        autoAdvanceTimer = 0.0f;
    }
}

void AetherwaveApp::updateUI() {
    float currentTime = ofGetElapsedTimef();
    float timeSinceInteraction = currentTime - lastInteractionTime;

    // Show UI shortly after interaction
    if (timeSinceInteraction < UI_FADE_DELAY) {
        showUI = true;
        uiOpacity = 1.0f;
    } else if (timeSinceInteraction < UI_FADE_DELAY + UI_FADE_DURATION) {
        // Fade out UI
        showUI = true;
        float fadeProgress = (timeSinceInteraction - UI_FADE_DELAY) / UI_FADE_DURATION;
        uiOpacity = 1.0f - fadeProgress;
    } else {
        // Hide UI completely
        showUI = false;
        uiOpacity = 0.0f;
    }
}

void AetherwaveApp::draw() {
    // Clear screen
    ofClear(0, 0, 0);

    if (showWelcomeScreen) {
        drawWelcomeScreen();
    } else {
        drawImageDisplay();
    }

    // Draw UI overlay
    if (showUI && uiOpacity > 0.0f) {
        drawUI();
    }

    // Draw performance overlay if enabled
    if (showPerformanceOverlay && performanceMonitor) {
        performanceMonitor->draw();
    }

    // Draw theme debug info if enabled
    if (showThemeDebug && themeManager) {
        themeManager->drawThemeDebugInfo(20, 100);
    }
}

void AetherwaveApp::drawWelcomeScreen() {
    // Calculate fade-in effect
    float fadeProgress = ofClamp(welcomeScreenTimer / 2.0f, 0.0f, 1.0f);
    float alpha = fadeProgress * 255;

    // Draw main title
    ofSetColor(255, 255, 255, alpha);
    string title = "AETHERWAVE";
    string subtitle = "Media Display Engine v2.0";
    string status = imageManager && imageManager->hasImages() ?
                   "Loading " + ofToString(imageManager->getImageCount()) + " images..." :
                   "Awaiting media files...";

    // Calculate centered positions
    auto titleBounds = ofGetStringBoundingBox(title, 0, 0);
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;

    // Draw title with larger font effect
    ofDrawBitmapString(title, centerX - titleBounds.width / 2.0f, centerY - 40);

    // Draw subtitle
    ofSetColor(200, 200, 200, alpha * 0.8f);
    auto subtitleBounds = ofGetStringBoundingBox(subtitle, 0, 0);
    ofDrawBitmapString(subtitle, centerX - subtitleBounds.width / 2.0f, centerY - 10);

    // Draw status
    ofSetColor(150, 150, 150, alpha * 0.6f);
    auto statusBounds = ofGetStringBoundingBox(status, 0, 0);
    ofDrawBitmapString(status, centerX - statusBounds.width / 2.0f, centerY + 20);

    // Draw progress indicator
    if (imageManager && imageManager->hasImages()) {
        float progress = std::min(welcomeScreenTimer / WELCOME_DURATION, 1.0f);
        float barWidth = 200;
        float barHeight = 4;
        float barX = centerX - barWidth / 2.0f;
        float barY = centerY + 60;

        // Background bar
        ofSetColor(50, 50, 50, alpha * 0.5f);
        ofDrawRectangle(barX, barY, barWidth, barHeight);

        // Progress bar
        ofSetColor(100, 150, 255, alpha);
        ofDrawRectangle(barX, barY, barWidth * progress, barHeight);
    }
}

void AetherwaveApp::drawImageDisplay() {
    if (imageManager) {
        imageManager->draw();
    } else {
        // Show "no images" message
        ofSetColor(100, 100, 100);
        string message = "No images loaded. Add images to assets/images/ directory.";
        auto bounds = ofGetStringBoundingBox(message, 0, 0);
        ofDrawBitmapString(message,
                          ofGetWidth() / 2.0f - bounds.width / 2.0f,
                          ofGetHeight() / 2.0f);
    }
}

void AetherwaveApp::drawUI() {
    ofPushStyle();

    // Apply UI opacity
    ofSetColor(255, 255, 255, uiOpacity * 200);

    // Draw image info
    if (imageManager && imageManager->hasImages()) {
        const auto* currentImage = imageManager->getCurrentImage();
        if (currentImage) {
            string imageInfo = currentImage->filename + " (" +
                             ofToString(currentImage->originalWidth) + "x" +
                             ofToString(currentImage->originalHeight) + ")";
            string indexInfo = ofToString(imageManager->getCurrentImageIndex() + 1) +
                             " / " + ofToString(imageManager->getImageCount());

            // Draw image info (bottom left)
            ofDrawBitmapString(imageInfo, 20, ofGetHeight() - 40);
            ofDrawBitmapString(indexInfo, 20, ofGetHeight() - 20);
        }
    }

    // Draw controls (bottom right)
    vector<string> controls = {
        "SPACE: Next image",
        "F: Toggle fullscreen",
        "P: Performance overlay",
        "T: Theme debug info",
        "H: Toggle help",
        "ESC: Exit"
    };

    float y = ofGetHeight() - 20 - (controls.size() * 15);
    for (const string& control : controls) {
        auto bounds = ofGetStringBoundingBox(control, 0, 0);
        ofDrawBitmapString(control, ofGetWidth() - bounds.width - 20, y);
        y += 15;
    }

    // Draw auto-advance progress
    if (autoAdvanceInterval > 0) {
        float progress = autoAdvanceTimer / autoAdvanceInterval;
        float barWidth = 200;
        float barHeight = 2;
        float barX = ofGetWidth() / 2.0f - barWidth / 2.0f;
        float barY = ofGetHeight() - 10;

        // Background
        ofSetColor(50, 50, 50, uiOpacity * 100);
        ofDrawRectangle(barX, barY, barWidth, barHeight);

        // Progress
        ofSetColor(100, 150, 255, uiOpacity * 150);
        ofDrawRectangle(barX, barY, barWidth * progress, barHeight);
    }

    ofPopStyle();
}

void AetherwaveApp::keyPressed(int key) {
    resetInteractionTimer();

    switch(key) {
        case ' ':
        case OF_KEY_RIGHT:
        case OF_KEY_DOWN:
            nextImage();
            break;

        case OF_KEY_LEFT:
        case OF_KEY_UP:
            previousImage();
            break;

        case 'f':
        case 'F':
            toggleFullscreen();
            break;

        case 'p':
        case 'P':
            showPerformanceOverlay = !showPerformanceOverlay;
            ofLogNotice("Aetherwave") << "Performance overlay: "
                                      << (showPerformanceOverlay ? "ON" : "OFF");
            break;

        case 't':
        case 'T':
            showThemeDebug = !showThemeDebug;
            ofLogNotice("Aetherwave") << "Theme debug: "
                                      << (showThemeDebug ? "ON" : "OFF");
            break;

        case 'h':
        case 'H':
            // Toggle help (future feature)
            ofLogNotice("Aetherwave") << "Help requested";
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            if (monitorManager) {
                int monitorId = key - '1';
                monitorManager->setTargetMonitor(monitorId);
            }
            break;

        case OF_KEY_ESC:
            ofLogNotice("Aetherwave") << "Application exit requested";
            ofExit();
            break;

        default:
            break;
    }
}

void AetherwaveApp::keyReleased(int key) {
    // Handle key release events if needed
}

void AetherwaveApp::mouseMoved(int x, int y) {
    resetInteractionTimer();
}

void AetherwaveApp::mousePressed(int x, int y, int button) {
    resetInteractionTimer();

    if (button == 0) { // Left click
        nextImage();
    } else if (button == 2) { // Right click
        previousImage();
    }
}

void AetherwaveApp::windowResized(int w, int h) {
    ofLogNotice("Aetherwave") << "Window resized to: " << w << "x" << h;
}

void AetherwaveApp::nextImage() {
    if (!imageManager || !imageManager->hasImages()) return;

    int nextIndex = (imageManager->getCurrentImageIndex() + 1) % imageManager->getImageCount();
    imageManager->startTransition(nextIndex);
    autoAdvanceTimer = 0.0f; // Reset auto-advance timer

    ofLogNotice("Aetherwave") << "Next image: " << nextIndex;
}

void AetherwaveApp::previousImage() {
    if (!imageManager || !imageManager->hasImages()) return;

    int prevIndex = imageManager->getCurrentImageIndex() - 1;
    if (prevIndex < 0) {
        prevIndex = imageManager->getImageCount() - 1;
    }
    imageManager->startTransition(prevIndex);
    autoAdvanceTimer = 0.0f; // Reset auto-advance timer

    ofLogNotice("Aetherwave") << "Previous image: " << prevIndex;
}

void AetherwaveApp::toggleFullscreen() {
    isFullscreen = !isFullscreen;
    ofSetFullscreen(isFullscreen);
    ofLogNotice("Aetherwave") << "Fullscreen toggled: " << (isFullscreen ? "ON" : "OFF");
}

void AetherwaveApp::resetInteractionTimer() {
    lastInteractionTime = ofGetElapsedTimef();
}

} // namespace Aetherwave

//========================================================================
int main() {
    ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);
    ofRunApp(new Aetherwave::AetherwaveApp());
    return 0;
}
