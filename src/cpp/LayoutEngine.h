#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <memory>

namespace Aetherwave {

/**
 * Phase 4B.1: Dynamic Layout Engine
 * 
 * Handles content-responsive layouts that adapt to:
 * - Window dimensions and aspect ratios
 * - Multi-monitor configurations  
 * - Content characteristics (portrait/landscape)
 * - Theme-aware layout selection
 */

enum class LayoutMode {
    SINGLE_FULLSCREEN,    // Single image: Full-screen with theme-based framing
    DUAL_SPLIT,          // Dual display: Split-screen with complementary images
    GALLERY_MOSAIC,      // Gallery mode: Mosaic layouts based on aspect ratios
    ADAPTIVE_AUTO        // Portrait/landscape adaptive switching
};

enum class AspectRatioType {
    PORTRAIT,    // Height > Width (9:16, 3:4, etc.)
    LANDSCAPE,   // Width > Height (16:9, 4:3, etc.)
    SQUARE       // Width ≈ Height (1:1)
};

struct DisplayInfo {
    SDL_Rect bounds;     // Display bounds (x, y, width, height)
    float dpiScale;      // DPI scale factor (relative to 96 DPI)
    float hdpi, vdpi;    // Horizontal and vertical DPI
    int refreshRate;     // Display refresh rate
    
    DisplayInfo() : bounds{0,0,0,0}, dpiScale(1.0f), hdpi(96.0f), vdpi(96.0f), refreshRate(60) {}
};

struct ImageLayoutInfo {
    int textureWidth;
    int textureHeight;
    float aspectRatio;
    AspectRatioType orientationType;
    SDL_Rect idealRect;       // Calculated ideal display rectangle
    float scaleFactor;        // Scale factor for this image
    bool isComplimentary;     // Whether this pairs well with other images
    
    ImageLayoutInfo() : textureWidth(0), textureHeight(0), aspectRatio(1.0f),
                       orientationType(AspectRatioType::SQUARE), idealRect{0,0,0,0},
                       scaleFactor(1.0f), isComplimentary(false) {}
};

struct LayoutConfiguration {
    LayoutMode mode;
    int windowWidth;
    int windowHeight;
    float windowAspectRatio;
    int maxImages;           // Maximum images to display simultaneously
    bool preserveAspectRatio;
    bool enableFraming;      // Theme-based framing around images
    float paddingPercent;    // Padding as percentage of window size
    
    LayoutConfiguration() : mode(LayoutMode::SINGLE_FULLSCREEN), windowWidth(1920), 
                          windowHeight(1080), windowAspectRatio(16.0f/9.0f), maxImages(1),
                          preserveAspectRatio(true), enableFraming(true), paddingPercent(0.05f) {}
};

class LayoutEngine {
private:
    LayoutConfiguration config;
    std::vector<ImageLayoutInfo> imageLayouts;
    
    // Window and monitor management with DPI awareness
    SDL_Window* window;
    int currentDisplayIndex;
    std::vector<DisplayInfo> availableDisplays;
    
    // Content analysis
    struct ContentAnalysis {
        int portraitCount;
        int landscapeCount; 
        int squareCount;
        float averageAspectRatio;
        bool hasMixedOrientations;
        
        ContentAnalysis() : portraitCount(0), landscapeCount(0), squareCount(0),
                          averageAspectRatio(1.0f), hasMixedOrientations(false) {}
    };
    
    ContentAnalysis contentStats;

public:
    LayoutEngine(SDL_Window* window);
    ~LayoutEngine() = default;
    
    // Core layout functionality
    void updateWindowDimensions(int width, int height);
    void setLayoutMode(LayoutMode mode);
    void analyzeContent(const std::vector<SDL_Texture*>& textures);
    void calculateLayout(const std::vector<SDL_Texture*>& textures);
    
    // Multi-monitor support
    void detectDisplays();
    void moveToDisplay(int displayIndex);
    void adaptToCurrentDisplay();
    bool isOnOptimalDisplay();
    
    // Layout queries
    LayoutMode getCurrentMode() const { return config.mode; }
    const std::vector<ImageLayoutInfo>& getImageLayouts() const { return imageLayouts; }
    SDL_Rect getImageRect(size_t imageIndex) const;
    bool shouldUseFraming() const { return config.enableFraming; }
    
    // Adaptive layout selection
    LayoutMode selectOptimalLayout();
    void enableAdaptiveMode(bool enable);
    
    // Debug and info
    void printLayoutInfo() const;
    void printDisplayInfo() const;

private:
    // Layout calculation methods
    void calculateSingleFullscreenLayout(const std::vector<SDL_Texture*>& textures);
    void calculateDualSplitLayout(const std::vector<SDL_Texture*>& textures);
    void calculateGalleryMosaicLayout(const std::vector<SDL_Texture*>& textures);
    void calculateAdaptiveLayout(const std::vector<SDL_Texture*>& textures);
    
    // Helper methods
    AspectRatioType analyzeAspectRatio(float ratio) const;
    SDL_Rect centerImageInBounds(int imgWidth, int imgHeight, SDL_Rect bounds, bool preserveAspect = true) const;
    SDL_Rect centerImageInBoundsWithDPI(int imgWidth, int imgHeight, SDL_Rect bounds, float dpiScale, bool preserveAspect = true) const;
    SDL_Rect addFramePadding(SDL_Rect rect, float paddingPercent) const;
    bool areImagesComplimentary(const ImageLayoutInfo& img1, const ImageLayoutInfo& img2) const;
    
    // Multi-monitor helpers
    void updateDisplayList();
    SDL_Rect getCurrentDisplayBounds() const;
    int findOptimalDisplay(int windowWidth, int windowHeight) const;
};

/**
 * Window Manager for handling resize events and multi-monitor coordination
 */
class WindowManager {
private:
    SDL_Window* window;
    LayoutEngine* layoutEngine;
    
    // Window state tracking  
    int lastWidth, lastHeight;
    int lastDisplayIndex;
    bool isFullscreen;
    bool wasMovedBetweenDisplays;

public:
    WindowManager(SDL_Window* window, LayoutEngine* layoutEngine);
    ~WindowManager() = default;
    
    // Event handling
    void handleWindowEvent(const SDL_WindowEvent& event);
    void handleResizeEvent(int newWidth, int newHeight);
    void handleMoveEvent();
    
    // Window controls
    void toggleFullscreen();
    void centerOnCurrentDisplay();
    void maximizeOnCurrentDisplay();
    
    // State queries
    bool wasRecentlyResized() const;
    bool wasRecentlyMoved() const; 
    SDL_Rect getCurrentWindowBounds() const;
    
private:
    void updateWindowState();
    void notifyLayoutEngine();
};

} // namespace Aetherwave