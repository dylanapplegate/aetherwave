#include "ofMain.h"
#include "AetherwaveApp.h"

namespace Aetherwave {

/**
 * Main application class for Aetherwave media display engine.
 * Provides full-screen window management and basic rendering loop.
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
    
private:
    // Application state
    bool isFullscreen;
    float startTime;
    
    // Constants
    static constexpr float FADE_DURATION = 2.0f;
    static constexpr int TARGET_FPS = 60;
};

void AetherwaveApp::setup() {
    // Initialize application
    ofSetFrameRate(TARGET_FPS);
    ofSetVerticalSync(true);
    ofBackground(0, 0, 0);
    
    // Set fullscreen for cinematic experience
    isFullscreen = true;
    ofSetFullscreen(true);
    
    // Store start time for animations
    startTime = ofGetElapsedTimef();
    
    ofLogNotice("Aetherwave") << "Application initialized - Fullscreen: " 
                               << (isFullscreen ? "ON" : "OFF");
}

void AetherwaveApp::update() {
    // Update application state
    // Future: Image loading, transitions, metadata processing
}

void AetherwaveApp::draw() {
    // Clear screen with black background
    ofClear(0, 0, 0);
    
    // Calculate fade-in effect for initial startup
    float elapsed = ofGetElapsedTimef() - startTime;
    float alpha = ofClamp(elapsed / FADE_DURATION, 0.0f, 1.0f);
    
    // Draw hello world text with fade-in
    ofSetColor(255, 255, 255, alpha * 255);
    
    string welcomeText = "Aetherwave Media Display Engine";
    string subtitle = "Press 'f' to toggle fullscreen | Press 'ESC' to exit";
    
    // Center text on screen
    ofRectangle textBounds = ofGetStringBoundingBox(welcomeText, 0, 0);
    float centerX = ofGetWidth() / 2.0f - textBounds.width / 2.0f;
    float centerY = ofGetHeight() / 2.0f;
    
    // Draw main title
    ofDrawBitmapString(welcomeText, centerX, centerY);
    
    // Draw subtitle
    ofRectangle subtitleBounds = ofGetStringBoundingBox(subtitle, 0, 0);
    float subtitleX = ofGetWidth() / 2.0f - subtitleBounds.width / 2.0f;
    ofSetColor(200, 200, 200, alpha * 200);
    ofDrawBitmapString(subtitle, subtitleX, centerY + 30);
    
    // Draw frame info in corner
    ofSetColor(100, 100, 100, alpha * 100);
    string frameInfo = "FPS: " + ofToString(ofGetFrameRate(), 1);
    ofDrawBitmapString(frameInfo, 20, ofGetHeight() - 20);
}

void AetherwaveApp::keyPressed(int key) {
    switch(key) {
        case 'f':
        case 'F':
            // Toggle fullscreen
            isFullscreen = !isFullscreen;
            ofSetFullscreen(isFullscreen);
            ofLogNotice("Aetherwave") << "Fullscreen toggled: " 
                                       << (isFullscreen ? "ON" : "OFF");
            break;
            
        case OF_KEY_ESC:
            // Exit application
            ofLogNotice("Aetherwave") << "Application exit requested";
            ofExit();
            break;
            
        default:
            break;
    }
}

void AetherwaveApp::keyReleased(int key) {
    // Handle key release events
    // Future: Media control, display mode switching
}

void AetherwaveApp::windowResized(int w, int h) {
    ofLogNotice("Aetherwave") << "Window resized to: " << w << "x" << h;
}

} // namespace Aetherwave

//========================================================================
int main() {
    ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);
    ofRunApp(new Aetherwave::AetherwaveApp());
    return 0;
}
