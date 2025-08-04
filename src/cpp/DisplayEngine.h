#pragma once

#include "ofMain.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

namespace Aetherwave {

/**
 * Advanced image management and display system.
 * Handles multiple image formats, preloading, and smooth transitions.
 */
class ImageManager {
public:
    struct ImageData {
        std::string filepath;
        std::string filename;
        ofImage image;
        bool loaded;
        float aspectRatio;
        int originalWidth;
        int originalHeight;

        ImageData() : loaded(false), aspectRatio(1.0f),
                     originalWidth(0), originalHeight(0) {}
    };

    struct DisplayState {
        int currentIndex;
        int nextIndex;
        float transitionProgress;
        bool isTransitioning;
        float transitionDuration;

        DisplayState() : currentIndex(-1), nextIndex(-1),
                        transitionProgress(0.0f), isTransitioning(false),
                        transitionDuration(2.0f) {}
    };

    ImageManager();
    ~ImageManager() = default;

    // Core functionality
    bool loadImagesFromDirectory(const std::string& directory);
    void preloadNextImages(int count = 3);
    void startTransition(int targetIndex);
    void update(float deltaTime);
    void draw();

    // Configuration
    void setTransitionDuration(float duration);
    void setTransitionType(const std::string& type);
    void setDisplayMode(const std::string& mode);

    // State queries
    int getCurrentImageIndex() const { return displayState.currentIndex; }
    int getImageCount() const { return static_cast<int>(images.size()); }
    bool hasImages() const { return !images.empty(); }
    bool isTransitioning() const { return displayState.isTransitioning; }

    // Image access
    const ImageData* getCurrentImage() const;
    const ImageData* getImageAt(int index) const;

private:
    std::vector<std::unique_ptr<ImageData>> images;
    DisplayState displayState;

    // Configuration
    std::string transitionType;
    std::string displayMode;

    // Supported file extensions
    static const std::vector<std::string> SUPPORTED_EXTENSIONS;

    // Helper methods
    bool isImageFile(const std::string& filepath) const;
    void loadImageAsync(ImageData* imageData);
    void drawTransition();
    void drawFadeTransition();
    void drawSlideTransition();
    void drawImage(const ImageData& imageData, float alpha = 1.0f);
    ofRectangle calculateImageBounds(const ImageData& imageData) const;
};

/**
 * Multi-monitor support and window management.
 */
class MonitorManager {
public:
    struct MonitorInfo {
        int id;
        int width;
        int height;
        int x;
        int y;
        bool isPrimary;

        MonitorInfo() : id(0), width(1920), height(1080),
                       x(0), y(0), isPrimary(true) {}
    };

    MonitorManager();
    ~MonitorManager() = default;

    // Monitor detection and management
    void detectMonitors();
    void setTargetMonitor(int monitorId);
    void enableMultiMonitor(bool enable);

    // Window positioning
    void positionWindow(int monitorId);
    void centerWindow();
    void maximizeWindow();

    // Queries
    int getMonitorCount() const { return static_cast<int>(monitors.size()); }
    const MonitorInfo* getPrimaryMonitor() const;
    const MonitorInfo* getMonitor(int id) const;

private:
    std::vector<MonitorInfo> monitors;
    int currentMonitorId;
    bool multiMonitorEnabled;

    void updateMonitorInfo();
};

/**
 * Performance monitoring and optimization.
 */
class PerformanceMonitor {
public:
    struct FrameStats {
        float averageFps;
        float minFps;
        float maxFps;
        float frameTime;
        int droppedFrames;

        FrameStats() : averageFps(60.0f), minFps(60.0f), maxFps(60.0f),
                      frameTime(16.67f), droppedFrames(0) {}
    };

    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    void update();
    void draw();

    const FrameStats& getStats() const { return stats; }
    bool isPerformanceGood() const;

private:
    FrameStats stats;
    std::vector<float> fpsHistory;
    float lastUpdateTime;
    int frameCount;

    static constexpr int FPS_HISTORY_SIZE = 60;
    static constexpr float MIN_ACCEPTABLE_FPS = 30.0f;

    void updateFpsHistory();
    void calculateStats();
};

} // namespace Aetherwave
