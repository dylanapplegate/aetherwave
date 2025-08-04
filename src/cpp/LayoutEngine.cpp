#include "LayoutEngine.h"
#include "DebugLogger.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace Aetherwave {

LayoutEngine::LayoutEngine(SDL_Window* window)
    : window(window), currentDisplayIndex(0) {

    detectDisplays();

    // Initialize with current window dimensions
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    updateWindowDimensions(w, h);

    std::cout << "🎨 Dynamic Layout Engine initialized" << std::endl;
    std::cout << "   Window: " << w << "x" << h << std::endl;
    std::cout << "   Displays detected: " << availableDisplays.size() << std::endl;
}

void LayoutEngine::updateWindowDimensions(int width, int height) {
    config.windowWidth = width;
    config.windowHeight = height;
    config.windowAspectRatio = (float)width / height;

    std::cout << "🔄 Layout engine updated for " << width << "x" << height
              << " (aspect: " << config.windowAspectRatio << ")" << std::endl;
}

void LayoutEngine::detectDisplays() {
    availableDisplays.clear();

    int numDisplays = SDL_GetNumVideoDisplays();

    for (int i = 0; i < numDisplays; i++) {
        DisplayInfo displayInfo;

        // Get display bounds
        if (SDL_GetDisplayBounds(i, &displayInfo.bounds) != 0) {
            continue; // Skip this display if we can't get bounds
        }

        // Get display DPI information
        float ddpi, hdpi, vdpi;
        if (SDL_GetDisplayDPI(i, &ddpi, &hdpi, &vdpi) == 0) {
            displayInfo.hdpi = hdpi;
            displayInfo.vdpi = vdpi;
            // Calculate DPI scale relative to standard 96 DPI
            displayInfo.dpiScale = ddpi / 96.0f;
        } else {
            // Fallback to default values if DPI detection fails
            displayInfo.hdpi = 96.0f;
            displayInfo.vdpi = 96.0f;
            displayInfo.dpiScale = 1.0f;
            std::cout << "⚠️ Could not detect DPI for display " << i << ", using defaults" << std::endl;
        }

        // Get display mode for refresh rate
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(i, &mode) == 0) {
            displayInfo.refreshRate = mode.refresh_rate;

            std::cout << "📺 Display " << i << ": " << displayInfo.bounds.w << "x" << displayInfo.bounds.h
                     << " at (" << displayInfo.bounds.x << "," << displayInfo.bounds.y << ") @ " << mode.refresh_rate << "Hz"
                     << " DPI: " << displayInfo.hdpi << " (scale: " << displayInfo.dpiScale << ")" << std::endl;
        }

        availableDisplays.push_back(displayInfo);
    }

    // Update current display index based on window position
    adaptToCurrentDisplay();
}

void LayoutEngine::adaptToCurrentDisplay() {
    if (availableDisplays.empty()) return;

    int windowX, windowY;
    SDL_GetWindowPosition(window, &windowX, &windowY);

    // Find which display contains the window center
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);
    int centerX = windowX + windowW / 2;
    int centerY = windowY + windowH / 2;

    DEBUG_WINDOW("POSITION_CHECK",
                 "Window at (" + std::to_string(windowX) + "," + std::to_string(windowY) + ") " +
                 "size " + std::to_string(windowW) + "x" + std::to_string(windowH) +
                 " center (" + std::to_string(centerX) + "," + std::to_string(centerY) + ")");

    for (size_t i = 0; i < availableDisplays.size(); i++) {
        const DisplayInfo& display = availableDisplays[i];

        DEBUG_DISPLAY("CHECK_BOUNDS",
                     "Display " + std::to_string(i) + " bounds: " +
                     std::to_string(display.bounds.x) + "," + std::to_string(display.bounds.y) + " " +
                     std::to_string(display.bounds.w) + "x" + std::to_string(display.bounds.h));

        if (centerX >= display.bounds.x && centerX < display.bounds.x + display.bounds.w &&
            centerY >= display.bounds.y && centerY < display.bounds.y + display.bounds.h) {

            if (currentDisplayIndex != static_cast<int>(i)) {
                DEBUG_WINDOW("DISPLAY_CHANGE",
                           "Changed from display " + std::to_string(currentDisplayIndex) +
                           " to display " + std::to_string(i));

                currentDisplayIndex = static_cast<int>(i);
                std::cout << "🖥️ Window moved to display " << i << " (" << display.bounds.w << "x" << display.bounds.h
                         << ", DPI scale: " << display.dpiScale << ")" << std::endl;

                DEBUG_DISPLAY("NEW_DISPLAY_INFO",
                            "Display " + std::to_string(i) + " size: " +
                            std::to_string(display.bounds.w) + "x" + std::to_string(display.bounds.h) +
                            " DPI scale: " + std::to_string(display.dpiScale));

                // Update layout for new display characteristics - use actual window dimensions
                updateWindowDimensions(windowW, windowH);
            } else {
                DEBUG_WINDOW("SAME_DISPLAY", "Still on display " + std::to_string(i));
            }
            break;
        }
    }
}

void LayoutEngine::setLayoutMode(LayoutMode mode) {
    if (config.mode != mode) {
        config.mode = mode;
        std::cout << "🎨 Layout mode changed to: ";

        switch (mode) {
            case LayoutMode::SINGLE_FULLSCREEN:
                std::cout << "Single Fullscreen";
                config.maxImages = 1;
                break;
            case LayoutMode::DUAL_SPLIT:
                std::cout << "Dual Split-Screen";
                config.maxImages = 2;
                break;
            case LayoutMode::GALLERY_MOSAIC:
                std::cout << "Gallery Mosaic";
                config.maxImages = 6; // Reasonable max for mosaic
                break;
            case LayoutMode::ADAPTIVE_AUTO:
                std::cout << "Adaptive Auto";
                config.maxImages = 4; // Will be adjusted based on content
                break;
        }
        std::cout << std::endl;
    }
}

void LayoutEngine::analyzeContent(const std::vector<SDL_Texture*>& textures) {
    contentStats = ContentAnalysis(); // Reset

    if (textures.empty()) return;

    float totalAspectRatio = 0.0f;

    for (size_t i = 0; i < textures.size() && i < imageLayouts.size(); i++) {
        if (!textures[i]) continue;

        int w, h;
        SDL_QueryTexture(textures[i], nullptr, nullptr, &w, &h);

        float aspectRatio = (float)w / h;
        totalAspectRatio += aspectRatio;

        AspectRatioType orientation = analyzeAspectRatio(aspectRatio);

        switch (orientation) {
            case AspectRatioType::PORTRAIT:
                contentStats.portraitCount++;
                break;
            case AspectRatioType::LANDSCAPE:
                contentStats.landscapeCount++;
                break;
            case AspectRatioType::SQUARE:
                contentStats.squareCount++;
                break;
        }
    }

    int totalImages = contentStats.portraitCount + contentStats.landscapeCount + contentStats.squareCount;
    if (totalImages > 0) {
        contentStats.averageAspectRatio = totalAspectRatio / totalImages;
        contentStats.hasMixedOrientations = (contentStats.portraitCount > 0 && contentStats.landscapeCount > 0);
    }

    std::cout << "📊 Content analysis: " << contentStats.portraitCount << " portrait, "
              << contentStats.landscapeCount << " landscape, " << contentStats.squareCount << " square" << std::endl;
}

AspectRatioType LayoutEngine::analyzeAspectRatio(float ratio) const {
    const float PORTRAIT_THRESHOLD = 0.85f;   // Below this = portrait
    const float LANDSCAPE_THRESHOLD = 1.15f;  // Above this = landscape

    if (ratio < PORTRAIT_THRESHOLD) {
        return AspectRatioType::PORTRAIT;
    } else if (ratio > LANDSCAPE_THRESHOLD) {
        return AspectRatioType::LANDSCAPE;
    } else {
        return AspectRatioType::SQUARE;
    }
}

void LayoutEngine::calculateLayout(const std::vector<SDL_Texture*>& textures) {
    static int callCount = 0;
    callCount++;

    // First, always check if we need to adapt to current display
    adaptToCurrentDisplay();

    // Get actual current window dimensions
    int actualWindowW, actualWindowH;
    SDL_GetWindowSize(window, &actualWindowW, &actualWindowH);

    // Update config if window dimensions have changed
    if (actualWindowW != config.windowWidth || actualWindowH != config.windowHeight) {
        std::cout << "🖥️ Window dimension change detected: " << config.windowWidth << "x" << config.windowHeight
                  << " -> " << actualWindowW << "x" << actualWindowH << std::endl;
        updateWindowDimensions(actualWindowW, actualWindowH);
    }

    // Create current state for comparison
    LayoutState currentState;
    currentState.windowWidth = config.windowWidth;
    currentState.windowHeight = config.windowHeight;
    currentState.textureCount = textures.size();
    currentState.mode = config.mode;
    currentState.displayIndex = currentDisplayIndex;

    // Capture texture dimensions
    currentState.textureDimensions.clear();
    for (const auto& texture : textures) {
        if (texture) {
            int w, h;
            SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
            currentState.textureDimensions.emplace_back(w, h);
        } else {
            currentState.textureDimensions.emplace_back(0, 0);
        }
    }

    // Check if we need to recalculate
    if (callCount > 1 && currentState == lastCalculatedState) {
        // State hasn't changed, no need to recalculate
        std::cout << "⏸️ Layout state unchanged, skipping recalculation" << std::endl;
        return;
    }

    // Debug: Print state comparison details
    if (callCount > 1) {
        std::cout << "🔄 Layout state changed:" << std::endl;
        std::cout << "   Window: " << lastCalculatedState.windowWidth << "x" << lastCalculatedState.windowHeight
                  << " -> " << currentState.windowWidth << "x" << currentState.windowHeight << std::endl;
        std::cout << "   Textures: " << lastCalculatedState.textureCount << " -> " << currentState.textureCount << std::endl;
        std::cout << "   Display: " << lastCalculatedState.displayIndex << " -> " << currentState.displayIndex << std::endl;
    }

    // Update the last calculated state
    lastCalculatedState = currentState;

    std::cout << "🎨 calculateLayout() call #" << callCount << " - textures: " << textures.size()
              << " window: " << config.windowWidth << "x" << config.windowHeight << std::endl;

    if (textures.empty()) {
        imageLayouts.clear();
        return;
    }

    // Resize layout info vector to match texture count
    size_t oldSize = imageLayouts.size();
    imageLayouts.resize(std::min(textures.size(), static_cast<size_t>(config.maxImages)));
    std::cout << "📦 Layout vector resized from " << oldSize << " to " << imageLayouts.size() << std::endl;    // Force content analysis since layout can change based on display/window changes
    analyzeContent(textures);

    // Calculate layouts based on current mode
    switch (config.mode) {
        case LayoutMode::SINGLE_FULLSCREEN:
            calculateSingleFullscreenLayout(textures);
            break;
        case LayoutMode::DUAL_SPLIT:
            calculateDualSplitLayout(textures);
            break;
        case LayoutMode::GALLERY_MOSAIC:
            calculateGalleryMosaicLayout(textures);
            break;
        case LayoutMode::ADAPTIVE_AUTO:
            calculateAdaptiveLayout(textures);
            break;
    }
}

void LayoutEngine::calculateSingleFullscreenLayout(const std::vector<SDL_Texture*>& textures) {
    if (textures.empty() || !textures[0]) return;

    int w, h;
    SDL_QueryTexture(textures[0], nullptr, nullptr, &w, &h);

    ImageLayoutInfo& layout = imageLayouts[0];
    layout.textureWidth = w;
    layout.textureHeight = h;
    layout.aspectRatio = (float)w / h;
    layout.orientationType = analyzeAspectRatio(layout.aspectRatio);

    // Calculate centered, aspect-preserving rectangle using DPI-aware method
    SDL_Rect windowBounds = { 0, 0, config.windowWidth, config.windowHeight };
    float dpiScale = 1.0f;
    if (currentDisplayIndex >= 0 && currentDisplayIndex < availableDisplays.size()) {
        dpiScale = availableDisplays[currentDisplayIndex].dpiScale;
    }
    layout.idealRect = centerImageInBoundsWithDPI(w, h, windowBounds, dpiScale, config.preserveAspectRatio);

    // Apply theme-based framing if enabled
    if (config.enableFraming) {
        layout.idealRect = addFramePadding(layout.idealRect, config.paddingPercent);
    }

    layout.scaleFactor = (float)layout.idealRect.w / w;
    layout.isComplimentary = true; // Single image is always "complimentary"
}

void LayoutEngine::calculateDualSplitLayout(const std::vector<SDL_Texture*>& textures) {
    if (textures.size() < 2) {
        // Fall back to single image layout
        calculateSingleFullscreenLayout(textures);
        return;
    }

    // Determine split orientation based on content and window aspect ratio
    bool verticalSplit = (config.windowAspectRatio > 1.4f); // Landscape window -> vertical split

    // If content has mixed orientations, choose split that works best
    if (contentStats.hasMixedOrientations) {
        verticalSplit = true; // Vertical split works better for mixed content
    }

    for (int i = 0; i < 2 && i < static_cast<int>(textures.size()); i++) {
        if (!textures[i]) continue;

        int w, h;
        SDL_QueryTexture(textures[i], nullptr, nullptr, &w, &h);

        ImageLayoutInfo& layout = imageLayouts[i];
        layout.textureWidth = w;
        layout.textureHeight = h;
        layout.aspectRatio = (float)w / h;
        layout.orientationType = analyzeAspectRatio(layout.aspectRatio);

        SDL_Rect halfBounds;
        if (verticalSplit) {
            // Split window vertically (side by side)
            halfBounds = {
                i * config.windowWidth / 2,
                0,
                config.windowWidth / 2,
                config.windowHeight
            };
        } else {
            // Split window horizontally (top and bottom)
            halfBounds = {
                0,
                i * config.windowHeight / 2,
                config.windowWidth,
                config.windowHeight / 2
            };
        }

        layout.idealRect = centerImageInBounds(w, h, halfBounds, config.preserveAspectRatio);

        // Add padding between images
        if (config.enableFraming) {
            layout.idealRect = addFramePadding(layout.idealRect, config.paddingPercent);
        }

        layout.scaleFactor = (float)layout.idealRect.w / w;
        layout.isComplimentary = (i == 1) ? areImagesComplimentary(imageLayouts[0], imageLayouts[1]) : true;
    }

    std::cout << "🖼️ Dual split layout: " << (verticalSplit ? "vertical" : "horizontal") << " split" << std::endl;
}

void LayoutEngine::calculateGalleryMosaicLayout(const std::vector<SDL_Texture*>& textures) {
    if (textures.empty()) return;

    size_t imageCount = std::min(textures.size(), static_cast<size_t>(config.maxImages));

    // Choose grid layout based on image count and window aspect ratio
    int cols, rows;
    if (imageCount <= 2) {
        cols = imageCount;
        rows = 1;
    } else if (imageCount <= 4) {
        cols = 2;
        rows = 2;
    } else if (imageCount <= 6) {
        cols = 3;
        rows = 2;
    } else {
        cols = 3;
        rows = 3;
    }

    // Adjust for window aspect ratio
    if (config.windowAspectRatio < 1.0f) { // Portrait window
        std::swap(cols, rows); // More rows than columns
    }

    int cellWidth = config.windowWidth / cols;
    int cellHeight = config.windowHeight / rows;

    for (size_t i = 0; i < imageCount; i++) {
        if (!textures[i]) continue;

        int w, h;
        SDL_QueryTexture(textures[i], nullptr, nullptr, &w, &h);

        ImageLayoutInfo& layout = imageLayouts[i];
        layout.textureWidth = w;
        layout.textureHeight = h;
        layout.aspectRatio = (float)w / h;
        layout.orientationType = analyzeAspectRatio(layout.aspectRatio);

        // Calculate cell position
        int col = static_cast<int>(i) % cols;
        int row = static_cast<int>(i) / cols;

        SDL_Rect cellBounds = {
            col * cellWidth,
            row * cellHeight,
            cellWidth,
            cellHeight
        };

        layout.idealRect = centerImageInBounds(w, h, cellBounds, config.preserveAspectRatio);

        // Add cell padding
        if (config.enableFraming) {
            layout.idealRect = addFramePadding(layout.idealRect, config.paddingPercent * 2); // Double padding for grid
        }

        layout.scaleFactor = (float)layout.idealRect.w / w;
        layout.isComplimentary = true; // In mosaic, all images are considered complementary
    }

    std::cout << "🖼️ Gallery mosaic layout: " << cols << "x" << rows << " grid for " << imageCount << " images" << std::endl;
}

void LayoutEngine::calculateAdaptiveLayout(const std::vector<SDL_Texture*>& textures) {
    // Select optimal layout mode based on content analysis and window characteristics
    LayoutMode optimalMode = selectOptimalLayout();

    // Directly calculate using optimal mode without recursion
    LayoutMode originalMode = config.mode;
    config.mode = optimalMode;

    switch (optimalMode) {
        case LayoutMode::SINGLE_FULLSCREEN:
            calculateSingleFullscreenLayout(textures);
            break;
        case LayoutMode::DUAL_SPLIT:
            calculateDualSplitLayout(textures);
            break;
        case LayoutMode::GALLERY_MOSAIC:
            calculateGalleryMosaicLayout(textures);
            break;
        default:
            calculateSingleFullscreenLayout(textures);
            break;
    }

    // Restore adaptive mode
    config.mode = originalMode;

    std::cout << "🎯 Adaptive layout selected: ";
    switch (optimalMode) {
        case LayoutMode::SINGLE_FULLSCREEN: std::cout << "Single Fullscreen"; break;
        case LayoutMode::DUAL_SPLIT: std::cout << "Dual Split"; break;
        case LayoutMode::GALLERY_MOSAIC: std::cout << "Gallery Mosaic"; break;
        default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;
}

LayoutMode LayoutEngine::selectOptimalLayout() {
    // Decision logic based on content analysis and window characteristics

    // If only one image or very few images, use single fullscreen
    if (contentStats.portraitCount + contentStats.landscapeCount + contentStats.squareCount <= 1) {
        return LayoutMode::SINGLE_FULLSCREEN;
    }

    // If exactly 2 images and they're complementary, use dual split
    if (contentStats.portraitCount + contentStats.landscapeCount + contentStats.squareCount == 2) {
        // Check if window is wide enough for dual split
        if (config.windowAspectRatio > 1.5f) {
            return LayoutMode::DUAL_SPLIT;
        }
    }

    // If mixed orientations and moderate image count, use dual split
    if (contentStats.hasMixedOrientations &&
        contentStats.portraitCount + contentStats.landscapeCount <= 4) {
        return LayoutMode::DUAL_SPLIT;
    }

    // For larger collections or uniform orientations, use gallery mosaic
    if (contentStats.portraitCount + contentStats.landscapeCount + contentStats.squareCount > 2) {
        return LayoutMode::GALLERY_MOSAIC;
    }

    // Default fallback
    return LayoutMode::SINGLE_FULLSCREEN;
}

SDL_Rect LayoutEngine::centerImageInBounds(int imgWidth, int imgHeight, SDL_Rect bounds, bool preserveAspect) const {
    // Use DPI-aware version with current display's DPI scale
    float dpiScale = 1.0f;
    if (currentDisplayIndex >= 0 && currentDisplayIndex < static_cast<int>(availableDisplays.size())) {
        dpiScale = availableDisplays[currentDisplayIndex].dpiScale;
    }

    return centerImageInBoundsWithDPI(imgWidth, imgHeight, bounds, dpiScale, preserveAspect);
}

SDL_Rect LayoutEngine::centerImageInBoundsWithDPI(int imgWidth, int imgHeight, SDL_Rect bounds, float dpiScale, bool preserveAspect) const {
    std::cout << "🔍 centerImageInBoundsWithDPI: img=" << imgWidth << "x" << imgHeight
              << " bounds=" << bounds.w << "x" << bounds.h << " at (" << bounds.x << "," << bounds.y << ")"
              << " dpiScale=" << dpiScale << std::endl;

    if (!preserveAspect) {
        return bounds; // Just stretch to fill bounds
    }

    // Use double precision for better accuracy
    double scaleX = static_cast<double>(bounds.w) / imgWidth;
    double scaleY = static_cast<double>(bounds.h) / imgHeight;
    double scale = std::min(scaleX, scaleY); // Preserve aspect ratio

    // Calculate scaled dimensions with high precision
    double scaledWidth = imgWidth * scale;
    double scaledHeight = imgHeight * scale;

    // Center within bounds with pixel-perfect positioning
    // Use DPI scale for sub-pixel precision on high-DPI displays
    double x = bounds.x + (bounds.w - scaledWidth) / 2.0;
    double y = bounds.y + (bounds.h - scaledHeight) / 2.0;

    // For high-DPI displays, ensure precise pixel alignment
    if (dpiScale > 1.0f) {
        x = std::round(x * dpiScale) / dpiScale;
        y = std::round(y * dpiScale) / dpiScale;
    }

    SDL_Rect result = {
        static_cast<int>(std::round(x)),
        static_cast<int>(std::round(y)),
        static_cast<int>(std::round(scaledWidth)),
        static_cast<int>(std::round(scaledHeight))
    };

    std::cout << "🔍 centerImageInBoundsWithDPI result: " << result.w << "x" << result.h
              << " at (" << result.x << "," << result.y << ")" << std::endl;

    return result;
}

SDL_Rect LayoutEngine::addFramePadding(SDL_Rect rect, float paddingPercent) const {
    int paddingX = static_cast<int>(config.windowWidth * paddingPercent);
    int paddingY = static_cast<int>(config.windowHeight * paddingPercent);

    return {
        rect.x + paddingX,
        rect.y + paddingY,
        rect.w - 2 * paddingX,
        rect.h - 2 * paddingY
    };
}

bool LayoutEngine::areImagesComplimentary(const ImageLayoutInfo& img1, const ImageLayoutInfo& img2) const {
    // Images are complementary if they have different orientations or similar aspect ratios
    if (img1.orientationType != img2.orientationType) {
        return true; // Different orientations work well together
    }

    // Similar aspect ratios also work well
    float aspectDiff = std::abs(img1.aspectRatio - img2.aspectRatio);
    return aspectDiff < 0.5f;
}

SDL_Rect LayoutEngine::getImageRect(size_t imageIndex) const {
    if (imageIndex >= imageLayouts.size()) {
        std::cout << "⚠️ getImageRect: Index " << imageIndex << " >= size " << imageLayouts.size() << std::endl;
        return { 0, 0, 0, 0 };
    }

    SDL_Rect rect = imageLayouts[imageIndex].idealRect;
    std::cout << "📐 getImageRect(" << imageIndex << "): " << rect.w << "x" << rect.h
              << " at (" << rect.x << "," << rect.y << ")" << std::endl;
    return rect;
}

void LayoutEngine::printLayoutInfo() const {
    std::cout << "\n🎨 Layout Engine Status:" << std::endl;
    std::cout << "   Mode: ";
    switch (config.mode) {
        case LayoutMode::SINGLE_FULLSCREEN: std::cout << "Single Fullscreen"; break;
        case LayoutMode::DUAL_SPLIT: std::cout << "Dual Split"; break;
        case LayoutMode::GALLERY_MOSAIC: std::cout << "Gallery Mosaic"; break;
        case LayoutMode::ADAPTIVE_AUTO: std::cout << "Adaptive Auto"; break;
    }
    std::cout << std::endl;

    std::cout << "   Window: " << config.windowWidth << "x" << config.windowHeight
              << " (aspect: " << config.windowAspectRatio << ")" << std::endl;
    std::cout << "   Display: " << currentDisplayIndex << " of " << availableDisplays.size() << std::endl;
    std::cout << "   Images: " << imageLayouts.size() << " layouts calculated" << std::endl;

    for (size_t i = 0; i < imageLayouts.size(); i++) {
        const auto& layout = imageLayouts[i];
        std::cout << "   Image " << i << ": " << layout.textureWidth << "x" << layout.textureHeight
                  << " -> " << layout.idealRect.w << "x" << layout.idealRect.h
                  << " at (" << layout.idealRect.x << "," << layout.idealRect.y << ")" << std::endl;
    }
}

void LayoutEngine::printDisplayInfo() const {
    std::cout << "\n📺 Display Information:" << std::endl;
    for (size_t i = 0; i < availableDisplays.size(); i++) {
        const DisplayInfo& display = availableDisplays[i];
        std::cout << "   Display " << i << ": " << display.bounds.w << "x" << display.bounds.h
                  << " at (" << display.bounds.x << "," << display.bounds.y << ")"
                  << " DPI: " << display.hdpi << " (scale: " << display.dpiScale << ")";
        if (static_cast<int>(i) == currentDisplayIndex) {
            std::cout << " (current)";
        }
        std::cout << std::endl;
    }
}

// WindowManager Implementation
WindowManager::WindowManager(SDL_Window* window, LayoutEngine* layoutEngine)
    : window(window), layoutEngine(layoutEngine), lastWidth(0), lastHeight(0),
      lastDisplayIndex(-1), isFullscreen(false), wasMovedBetweenDisplays(false) {

    updateWindowState();
    std::cout << "🪟 Window Manager initialized" << std::endl;
}

void WindowManager::handleWindowEvent(const SDL_WindowEvent& event) {
    switch (event.event) {
        case SDL_WINDOWEVENT_RESIZED:
            handleResizeEvent(event.data1, event.data2);
            break;

        case SDL_WINDOWEVENT_MOVED:
            handleMoveEvent();
            break;

        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_RESTORED:
            updateWindowState();
            notifyLayoutEngine();
            break;
    }
}

void WindowManager::handleResizeEvent(int newWidth, int newHeight) {
    if (newWidth != lastWidth || newHeight != lastHeight) {
        std::cout << "🔄 Window resized: " << lastWidth << "x" << lastHeight
                  << " -> " << newWidth << "x" << newHeight << std::endl;

        lastWidth = newWidth;
        lastHeight = newHeight;

        notifyLayoutEngine();
    }
}

void WindowManager::handleMoveEvent() {
    layoutEngine->adaptToCurrentDisplay();

    // Check if we moved between displays
    int currentDisplay = -1;
    int windowX, windowY;
    SDL_GetWindowPosition(window, &windowX, &windowY);

    for (int i = 0; i < SDL_GetNumVideoDisplays(); i++) {
        SDL_Rect displayBounds;
        if (SDL_GetDisplayBounds(i, &displayBounds) == 0) {
            if (windowX >= displayBounds.x && windowX < displayBounds.x + displayBounds.w) {
                currentDisplay = i;
                break;
            }
        }
    }

    if (currentDisplay != lastDisplayIndex && lastDisplayIndex != -1) {
        wasMovedBetweenDisplays = true;
        std::cout << "🖥️ Window moved from display " << lastDisplayIndex
                  << " to display " << currentDisplay << std::endl;
        notifyLayoutEngine();
    }

    lastDisplayIndex = currentDisplay;
}

void WindowManager::updateWindowState() {
    SDL_GetWindowSize(window, &lastWidth, &lastHeight);

    Uint32 flags = SDL_GetWindowFlags(window);
    isFullscreen = (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void WindowManager::notifyLayoutEngine() {
    if (layoutEngine) {
        layoutEngine->updateWindowDimensions(lastWidth, lastHeight);
    }
}

void WindowManager::toggleFullscreen() {
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, 0);
        isFullscreen = false;
        std::cout << "🪟 Switched to windowed mode" << std::endl;
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        isFullscreen = true;
        std::cout << "🖥️ Switched to fullscreen mode" << std::endl;
    }

    updateWindowState();
    notifyLayoutEngine();
}

bool WindowManager::wasRecentlyResized() const {
    // This would be implemented with timestamp tracking if needed
    return false;
}

bool WindowManager::wasRecentlyMoved() const {
    bool result = wasMovedBetweenDisplays;
    const_cast<WindowManager*>(this)->wasMovedBetweenDisplays = false; // Reset flag
    return result;
}

SDL_Rect WindowManager::getCurrentWindowBounds() const {
    int x, y, w, h;
    SDL_GetWindowPosition(window, &x, &y);
    SDL_GetWindowSize(window, &w, &h);
    return { x, y, w, h };
}

} // namespace Aetherwave