#include "DisplayEngine.h"
#include "ThemeManager.h"
#include "ofMain.h"
#include <filesystem>
#include <algorithm>

namespace Aetherwave {

// Static initialization
const std::vector<std::string> ImageManager::SUPPORTED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".tiff", ".tif", ".webp", ".bmp"
};

//=============================================================================
// ImageManager Implementation
//=============================================================================

ImageManager::ImageManager()
    : transitionType("fade")
    , displayMode("fit")
    , themeManager_(nullptr)
{
    displayState = DisplayState();
}

bool ImageManager::loadImagesFromDirectory(const std::string& directory) {
    images.clear();
    displayState = DisplayState();

    try {
        if (!std::filesystem::exists(directory)) {
            ofLogError("ImageManager") << "Directory does not exist: " << directory;
            return false;
        }

        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();

                if (isImageFile(filepath)) {
                    auto imageData = std::make_unique<ImageData>();
                    imageData->filepath = filepath;
                    imageData->filename = entry.path().filename().string();

                    images.push_back(std::move(imageData));
                }
            }
        }

        // Sort images by filename for consistent ordering
        std::sort(images.begin(), images.end(),
            [](const std::unique_ptr<ImageData>& a, const std::unique_ptr<ImageData>& b) {
                return a->filename < b->filename;
            });

        ofLogNotice("ImageManager") << "Found " << images.size() << " images in " << directory;

        // Load first image immediately
        if (!images.empty()) {
            displayState.currentIndex = 0;
            loadImageAsync(images[0].get());
            preloadNextImages(2);
        }

        return !images.empty();
    }
    catch (const std::exception& e) {
        ofLogError("ImageManager") << "Error loading images: " << e.what();
        return false;
    }
}

void ImageManager::preloadNextImages(int count) {
    if (images.empty()) return;

    int startIndex = displayState.currentIndex + 1;
    for (int i = 0; i < count; ++i) {
        int index = (startIndex + i) % static_cast<int>(images.size());
        if (index < static_cast<int>(images.size()) && !images[index]->loaded) {
            loadImageAsync(images[index].get());
        }
    }
}

void ImageManager::loadImageAsync(ImageData* imageData) {
    if (!imageData || imageData->loaded) return;

    try {
        bool success = imageData->image.load(imageData->filepath);
        if (success) {
            imageData->loaded = true;
            imageData->originalWidth = imageData->image.getWidth();
            imageData->originalHeight = imageData->image.getHeight();
            imageData->aspectRatio = static_cast<float>(imageData->originalWidth) /
                                   static_cast<float>(imageData->originalHeight);

            ofLogNotice("ImageManager") << "Loaded: " << imageData->filename
                                        << " (" << imageData->originalWidth
                                        << "x" << imageData->originalHeight << ")";
        } else {
            ofLogError("ImageManager") << "Failed to load: " << imageData->filepath;
        }
    }
    catch (const std::exception& e) {
        ofLogError("ImageManager") << "Exception loading " << imageData->filepath << ": " << e.what();
    }
}

void ImageManager::startTransition(int targetIndex) {
    if (images.empty() || targetIndex < 0 || targetIndex >= static_cast<int>(images.size())) {
        return;
    }

    if (targetIndex == displayState.currentIndex) {
        return; // Already displaying this image
    }

    displayState.nextIndex = targetIndex;
    displayState.isTransitioning = true;
    displayState.transitionProgress = 0.0f;

    // Ensure target image is loaded
    if (!images[targetIndex]->loaded) {
        loadImageAsync(images[targetIndex].get());
    }

    ofLogNotice("ImageManager") << "Starting transition to image " << targetIndex;
}

void ImageManager::update(float deltaTime) {
    if (!displayState.isTransitioning) return;

    displayState.transitionProgress += deltaTime / displayState.transitionDuration;

    if (displayState.transitionProgress >= 1.0f) {
        // Transition complete
        displayState.transitionProgress = 1.0f;
        displayState.currentIndex = displayState.nextIndex;
        displayState.nextIndex = -1;
        displayState.isTransitioning = false;

        // Preload next images
        preloadNextImages(2);

        ofLogNotice("ImageManager") << "Transition complete to image " << displayState.currentIndex;
    }
}

void ImageManager::draw() {
    if (images.empty()) return;

    if (displayState.isTransitioning) {
        drawTransition();
    } else if (displayState.currentIndex >= 0) {
        const auto* currentImage = getCurrentImage();
        if (currentImage && currentImage->loaded) {
            drawImage(*currentImage);
        }
    }
}

void ImageManager::drawTransition() {
    // Use theme-appropriate transition if theme manager is available
    std::string effectiveTransition = transitionType;
    if (themeManager_ && themeManager_->hasActiveTheme()) {
        effectiveTransition = selectThemeAppropriateTransition();
    }

    if (effectiveTransition == "fade") {
        drawFadeTransition();
    } else if (effectiveTransition == "slide") {
        drawSlideTransition();
    } else if (effectiveTransition == "glitch") {
        drawGlitchTransition();
    } else if (effectiveTransition == "dissolve") {
        drawDissolveTransition();
    } else if (effectiveTransition == "pixel") {
        drawPixelTransition();
    } else if (effectiveTransition == "soft_fade") {
        drawSoftFadeTransition();
    } else {
        drawFadeTransition(); // Default fallback
    }
}

void ImageManager::drawFadeTransition() {
    // Draw current image with decreasing alpha
    if (displayState.currentIndex >= 0) {
        const auto* currentImage = getCurrentImage();
        if (currentImage && currentImage->loaded) {
            float alpha = 1.0f - displayState.transitionProgress;
            drawImage(*currentImage, alpha);
        }
    }

    // Draw next image with increasing alpha
    if (displayState.nextIndex >= 0) {
        const auto* nextImage = getImageAt(displayState.nextIndex);
        if (nextImage && nextImage->loaded) {
            float alpha = displayState.transitionProgress;
            drawImage(*nextImage, alpha);
        }
    }
}

void ImageManager::drawSlideTransition() {
    // Simple slide transition implementation
    // This is a basic version - can be enhanced with easing

    ofPushMatrix();

    float slideOffset = ofGetWidth() * displayState.transitionProgress;

    // Draw current image sliding out
    if (displayState.currentIndex >= 0) {
        const auto* currentImage = getCurrentImage();
        if (currentImage && currentImage->loaded) {
            ofTranslate(-slideOffset, 0);
            drawImage(*currentImage);
        }
    }

    ofPopMatrix();
    ofPushMatrix();

    // Draw next image sliding in
    if (displayState.nextIndex >= 0) {
        const auto* nextImage = getImageAt(displayState.nextIndex);
        if (nextImage && nextImage->loaded) {
            ofTranslate(ofGetWidth() - slideOffset, 0);
            drawImage(*nextImage);
        }
    }

    ofPopMatrix();
}

void ImageManager::drawGlitchTransition() {
    // Cyberfemme-style glitch transition with chromatic aberration and digital artifacts
    float progress = displayState.transitionProgress;
    float glitchIntensity = sin(progress * OF_PI) * 20.0f;  // Peak in middle of transition

    // Draw current image with glitch effects
    if (displayState.currentIndex >= 0) {
        const auto* currentImage = getCurrentImage();
        if (currentImage && currentImage->loaded) {
            ofPushMatrix();

            // Add random horizontal offsets for glitch bars
            if (ofRandom(1.0f) > 0.7f) {
                float randomOffset = ofRandom(-glitchIntensity, glitchIntensity);
                ofTranslate(randomOffset, ofRandom(-5, 5));
            }

            // Fade out with glitch
            float alpha = (1.0f - progress) * (1.0f - glitchIntensity * 0.01f);

            // Chromatic aberration effect - draw color channels slightly offset
            ofSetColor(255, 0, 0, alpha * 255 * 0.8f);  // Red channel
            currentImage->image.draw(-2, 0, ofGetWidth(), ofGetHeight());

            ofSetColor(0, 255, 0, alpha * 255);  // Green channel (normal)
            currentImage->image.draw(0, 0, ofGetWidth(), ofGetHeight());

            ofSetColor(0, 0, 255, alpha * 255 * 0.8f);  // Blue channel
            currentImage->image.draw(2, 0, ofGetWidth(), ofGetHeight());

            ofPopMatrix();
        }
    }

    // Draw next image with glitch entry
    if (displayState.nextIndex >= 0) {
        const auto* nextImage = getImageAt(displayState.nextIndex);
        if (nextImage && nextImage->loaded) {
            ofPushMatrix();

            // Add digital noise effect
            if (progress > 0.3f && ofRandom(1.0f) > 0.8f) {
                ofSetColor(255, 0, 255, 50);  // Magenta noise
                ofDrawRectangle(0, ofRandom(ofGetHeight()),
                              ofGetWidth(), ofRandom(5, 20));
            }

            float alpha = progress;
            ofSetColor(255, 255, 255, alpha * 255);
            nextImage->image.draw(0, 0, ofGetWidth(), ofGetHeight());

            ofPopMatrix();
        }
    }

    // Add screen-wide glitch lines
    if (ofRandom(1.0f) > 0.9f) {
        ofSetColor(0, 255, 255, 100);  // Cyan glitch lines
        float y = ofRandom(ofGetHeight());
        ofDrawRectangle(0, y, ofGetWidth(), 2);
    }
}

void ImageManager::drawDissolveTransition() {
    // Digital dissolve with pixelated effect for tech themes
    float progress = displayState.transitionProgress;
    int pixelSize = 4;  // Size of dissolve pixels

    // Create dissolve pattern
    for (int x = 0; x < ofGetWidth(); x += pixelSize) {
        for (int y = 0; y < ofGetHeight(); y += pixelSize) {
            float dissolveThreshold = ofNoise(x * 0.01f, y * 0.01f, ofGetElapsedTimef() * 2.0f);

            if (dissolveThreshold < progress) {
                // Show next image
                if (displayState.nextIndex >= 0) {
                    const auto* nextImage = getImageAt(displayState.nextIndex);
                    if (nextImage && nextImage->loaded) {
                        ofSetColor(255);
                        nextImage->image.drawSubsection(x, y, pixelSize, pixelSize,
                                                      x, y, pixelSize, pixelSize);
                    }
                }
            } else {
                // Show current image
                if (displayState.currentIndex >= 0) {
                    const auto* currentImage = getCurrentImage();
                    if (currentImage && currentImage->loaded) {
                        ofSetColor(255);
                        currentImage->image.drawSubsection(x, y, pixelSize, pixelSize,
                                                         x, y, pixelSize, pixelSize);
                    }
                }
            }
        }
    }
}

void ImageManager::drawPixelTransition() {
    // Sharp pixel-based transition for tech themes
    float progress = displayState.transitionProgress;
    int blockSize = 8;

    for (int x = 0; x < ofGetWidth(); x += blockSize) {
        for (int y = 0; y < ofGetHeight(); y += blockSize) {
            // Use deterministic pattern based on position
            float threshold = ofNoise(x * 0.02f, y * 0.02f) * 1.2f - 0.1f;

            if (progress > threshold) {
                // Show next image
                if (displayState.nextIndex >= 0) {
                    const auto* nextImage = getImageAt(displayState.nextIndex);
                    if (nextImage && nextImage->loaded) {
                        ofSetColor(255);
                        nextImage->image.drawSubsection(x, y, blockSize, blockSize,
                                                      x, y, blockSize, blockSize);
                    }
                }
            } else {
                // Show current image
                if (displayState.currentIndex >= 0) {
                    const auto* currentImage = getCurrentImage();
                    if (currentImage && currentImage->loaded) {
                        ofSetColor(255);
                        currentImage->image.drawSubsection(x, y, blockSize, blockSize,
                                                         x, y, blockSize, blockSize);
                    }
                }
            }
        }
    }
}

void ImageManager::drawSoftFadeTransition() {
    // Gentle organic fade with soft edges for organic themes
    float progress = displayState.transitionProgress;

    // Use eased progress for more organic feel
    float easedProgress = progress * progress * (3.0f - 2.0f * progress); // Smoothstep

    // Draw current image
    if (displayState.currentIndex >= 0) {
        const auto* currentImage = getCurrentImage();
        if (currentImage && currentImage->loaded) {
            float alpha = 1.0f - easedProgress;
            ofSetColor(255, 255, 255, alpha * 255);
            currentImage->image.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
    }

    // Draw next image with soft entrance
    if (displayState.nextIndex >= 0) {
        const auto* nextImage = getImageAt(displayState.nextIndex);
        if (nextImage && nextImage->loaded) {
            float alpha = easedProgress;

            // Add subtle warmth to the transition
            if (themeManager_ && themeManager_->hasActiveTheme()) {
                ofColor warmTint = themeManager_->getAccentColor(0);
                warmTint.a = alpha * 20; // Very subtle tint
                ofSetColor(warmTint);
                ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            }

            ofSetColor(255, 255, 255, alpha * 255);
            nextImage->image.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
    }
}

void ImageManager::drawImage(const ImageData& imageData, float alpha) {
    if (!imageData.loaded) return;

    ofRectangle bounds = calculateImageBounds(imageData);

    ofSetColor(255, 255, 255, alpha * 255);
    imageData.image.draw(bounds.x, bounds.y, bounds.width, bounds.height);
}

ofRectangle ImageManager::calculateImageBounds(const ImageData& imageData) const {
    float screenWidth = static_cast<float>(ofGetWidth());
    float screenHeight = static_cast<float>(ofGetHeight());
    float screenAspect = screenWidth / screenHeight;

    float imageWidth = static_cast<float>(imageData.originalWidth);
    float imageHeight = static_cast<float>(imageData.originalHeight);

    ofRectangle bounds;

    if (displayMode == "fit") {
        // Fit image within screen, maintaining aspect ratio
        if (imageData.aspectRatio > screenAspect) {
            // Image is wider - fit to width
            bounds.width = screenWidth;
            bounds.height = screenWidth / imageData.aspectRatio;
            bounds.x = 0;
            bounds.y = (screenHeight - bounds.height) / 2.0f;
        } else {
            // Image is taller - fit to height
            bounds.height = screenHeight;
            bounds.width = screenHeight * imageData.aspectRatio;
            bounds.x = (screenWidth - bounds.width) / 2.0f;
            bounds.y = 0;
        }
    } else if (displayMode == "fill") {
        // Fill screen, crop if necessary
        if (imageData.aspectRatio > screenAspect) {
            // Image is wider - fit to height and crop sides
            bounds.height = screenHeight;
            bounds.width = screenHeight * imageData.aspectRatio;
            bounds.x = (screenWidth - bounds.width) / 2.0f;
            bounds.y = 0;
        } else {
            // Image is taller - fit to width and crop top/bottom
            bounds.width = screenWidth;
            bounds.height = screenWidth / imageData.aspectRatio;
            bounds.x = 0;
            bounds.y = (screenHeight - bounds.height) / 2.0f;
        }
    } else { // stretch
        bounds.set(0, 0, screenWidth, screenHeight);
    }

    return bounds;
}

const ImageManager::ImageData* ImageManager::getCurrentImage() const {
    if (displayState.currentIndex >= 0 &&
        displayState.currentIndex < static_cast<int>(images.size())) {
        return images[displayState.currentIndex].get();
    }
    return nullptr;
}

const ImageManager::ImageData* ImageManager::getImageAt(int index) const {
    if (index >= 0 && index < static_cast<int>(images.size())) {
        return images[index].get();
    }
    return nullptr;
}

bool ImageManager::isImageFile(const std::string& filepath) const {
    std::string extension = std::filesystem::path(filepath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    return std::find(SUPPORTED_EXTENSIONS.begin(), SUPPORTED_EXTENSIONS.end(), extension)
           != SUPPORTED_EXTENSIONS.end();
}

void ImageManager::setTransitionDuration(float duration) {
    displayState.transitionDuration = std::max(0.1f, duration);
}

void ImageManager::setTransitionType(const std::string& type) {
    transitionType = type;
}

void ImageManager::setDisplayMode(const std::string& mode) {
    displayMode = mode;
}

void ImageManager::setThemeManager(ThemeManager* themeManager) {
    themeManager_ = themeManager;
    ofLogNotice("ImageManager") << "Theme manager connected";
}

std::string ImageManager::selectThemeAppropriateTransition() const {
    if (!themeManager_ || !themeManager_->hasActiveTheme()) {
        return transitionType; // Fallback to manual setting
    }

    std::string preferredTransition = themeManager_->getPreferredTransition();

    // Map theme styles to available transitions
    if (preferredTransition == "glitch") {
        return "glitch";
    } else if (preferredTransition == "soft_fade") {
        return "soft_fade";
    } else if (preferredTransition == "pixel") {
        return "pixel";
    } else if (preferredTransition == "dissolve") {
        return "dissolve";
    }

    // Fallback based on theme name if preferred transition not recognized
    const auto& currentTheme = themeManager_->getCurrentTheme();
    if (currentTheme.themeName == "cyberfemme") {
        return "glitch";
    } else if (currentTheme.themeName == "organic") {
        return "soft_fade";
    } else if (currentTheme.themeName == "tech") {
        return "pixel";
    }

    return "fade"; // Final fallback
}

//=============================================================================
// MonitorManager Implementation
//=============================================================================

MonitorManager::MonitorManager()
    : currentMonitorId(0)
    , multiMonitorEnabled(false)
{
    detectMonitors();
}

void MonitorManager::detectMonitors() {
    monitors.clear();

    // Get primary monitor info
    MonitorInfo primary;
    primary.id = 0;
    primary.width = ofGetScreenWidth();
    primary.height = ofGetScreenHeight();
    primary.x = 0;
    primary.y = 0;
    primary.isPrimary = true;

    monitors.push_back(primary);

    ofLogNotice("MonitorManager") << "Detected " << monitors.size() << " monitor(s)";
    ofLogNotice("MonitorManager") << "Primary: " << primary.width << "x" << primary.height;
}

void MonitorManager::setTargetMonitor(int monitorId) {
    if (monitorId >= 0 && monitorId < static_cast<int>(monitors.size())) {
        currentMonitorId = monitorId;
        positionWindow(monitorId);
    }
}

void MonitorManager::positionWindow(int monitorId) {
    if (monitorId >= 0 && monitorId < static_cast<int>(monitors.size())) {
        const auto& monitor = monitors[monitorId];
        ofSetWindowPosition(monitor.x, monitor.y);
        ofSetWindowShape(monitor.width, monitor.height);

        ofLogNotice("MonitorManager") << "Positioned window on monitor " << monitorId;
    }
}

const MonitorManager::MonitorInfo* MonitorManager::getPrimaryMonitor() const {
    for (const auto& monitor : monitors) {
        if (monitor.isPrimary) {
            return &monitor;
        }
    }
    return monitors.empty() ? nullptr : &monitors[0];
}

const MonitorManager::MonitorInfo* MonitorManager::getMonitor(int id) const {
    for (const auto& monitor : monitors) {
        if (monitor.id == id) {
            return &monitor;
        }
    }
    return nullptr;
}

//=============================================================================
// PerformanceMonitor Implementation
//=============================================================================

PerformanceMonitor::PerformanceMonitor()
    : lastUpdateTime(0.0f)
    , frameCount(0)
{
    fpsHistory.reserve(FPS_HISTORY_SIZE);
}

void PerformanceMonitor::update() {
    float currentTime = ofGetElapsedTimef();
    frameCount++;

    if (currentTime - lastUpdateTime >= 1.0f) {
        updateFpsHistory();
        calculateStats();
        lastUpdateTime = currentTime;
        frameCount = 0;
    }
}

void PerformanceMonitor::updateFpsHistory() {
    float currentFps = ofGetFrameRate();

    fpsHistory.push_back(currentFps);
    if (fpsHistory.size() > FPS_HISTORY_SIZE) {
        fpsHistory.erase(fpsHistory.begin());
    }
}

void PerformanceMonitor::calculateStats() {
    if (fpsHistory.empty()) return;

    float sum = 0.0f;
    stats.minFps = fpsHistory[0];
    stats.maxFps = fpsHistory[0];

    for (float fps : fpsHistory) {
        sum += fps;
        stats.minFps = std::min(stats.minFps, fps);
        stats.maxFps = std::max(stats.maxFps, fps);
    }

    stats.averageFps = sum / static_cast<float>(fpsHistory.size());
    stats.frameTime = 1000.0f / stats.averageFps; // milliseconds
}

bool PerformanceMonitor::isPerformanceGood() const {
    return stats.averageFps >= MIN_ACCEPTABLE_FPS;
}

void PerformanceMonitor::draw() {
    // Simple performance overlay
    ofSetColor(255, 255, 255, 180);

    std::string perfText = "FPS: " + ofToString(stats.averageFps, 1) +
                          " | Frame: " + ofToString(stats.frameTime, 2) + "ms";

    if (!isPerformanceGood()) {
        ofSetColor(255, 100, 100, 180); // Red for poor performance
        perfText += " (LOW)";
    }

    ofDrawBitmapString(perfText, 20, ofGetHeight() - 40);
}

} // namespace Aetherwave
