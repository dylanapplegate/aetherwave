// Aetherwave Real Visual Display Engine using SDL2
// This creates an actual graphical window with image rendering
// Now includes theme-aware transitions and Python API integration

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "ThemeManagerSDL.h"
#include "LayoutEngine.h"
#include "DebugLogger.h"

namespace Aetherwave {

class VisualDisplayEngine {
private:
    // SDL Components
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Theme Management
    std::unique_ptr<Aetherwave::ThemeManagerSDL> themeManager;
    bool showThemeDebug = false;

    // Phase 4B.1: Dynamic Layout Engine
    std::unique_ptr<Aetherwave::LayoutEngine> layoutEngine;
    std::unique_ptr<Aetherwave::WindowManager> windowManager;
    bool showLayoutDebug = false;

    // Application State
    std::vector<std::string> imagePaths;
    std::vector<SDL_Texture*> imageTextures;
    size_t currentIndex = 0;
    bool isRunning = true;
    bool isFullscreen = false;

    // Window Properties
    int windowWidth = 1920;
    int windowHeight = 1080;

    // Cached Layout System
    SDL_Rect cachedImageRect = {0, 0, 0, 0};
    size_t cachedImageIndex = SIZE_MAX; // Invalid index initially
    bool layoutNeedsRecalc = true;

    // Transition layout cache
    SDL_Rect cachedTransitionCurrentRect = {0, 0, 0, 0};
    SDL_Rect cachedTransitionNextRect = {0, 0, 0, 0};
    size_t cachedTransitionCurrentIndex = SIZE_MAX;
    size_t cachedTransitionNextIndex = SIZE_MAX;

    // Enhanced Transition State
    enum class TransitionType {
        FADE,
        GLITCH,
        SOFT_FADE,
        PIXEL,
        DISSOLVE
    };

    TransitionType currentTransitionType = TransitionType::FADE;
    float fadeAlpha = 1.0f;
    bool isTransitioning = false;
    std::chrono::steady_clock::time_point transitionStart;

    // Glitch effect state
    std::mt19937 glitchRng;
    float glitchIntensity = 0.0f;

    // Random number generator
    std::random_device rd;

public:
    bool initialize() {
        std::cout << "🌊 Initializing Aetherwave Visual Display Engine..." << std::endl;

        // Initialize debug logging system first
        DebugLogger::getInstance().setLogFile("aetherwave_debug.log");
        DebugLogger::getInstance().enableConsoleOutput(true);
        DEBUG_LOG("SYSTEM", "Aetherwave Visual Display Engine starting up");

        // Initialize random number generator
        glitchRng.seed(rd());

        // Initialize theme manager
        themeManager = std::make_unique<Aetherwave::ThemeManagerSDL>();

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "❌ SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Set high-quality scaling hints for better image quality
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");  // Anisotropic filtering
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");     // Use OpenGL for better quality
        SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0"); // Prevent screensaver

        // Initialize SDL_image
        int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "❌ SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            return false;
        }

        // Get display info for optimal window size
        SDL_DisplayMode displayMode;
        if (SDL_GetDesktopDisplayMode(0, &displayMode) == 0) {
            windowWidth = displayMode.w;
            windowHeight = displayMode.h;
            std::cout << "📺 Display: " << windowWidth << "x" << windowHeight << std::endl;
        }

        // Create window
        window = SDL_CreateWindow(
            "Aetherwave Display Engine",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            windowWidth,
            windowHeight,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        );

        if (window == nullptr) {
            std::cerr << "❌ Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create renderer with hardware acceleration
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            std::cerr << "❌ Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Enable high-quality texture filtering for better scaling
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");  // Apply to renderer

        // Set blend mode for alpha transparency
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Initialize Phase 4B.1: Dynamic Layout Engine
        layoutEngine = std::make_unique<Aetherwave::LayoutEngine>(window);
        windowManager = std::make_unique<Aetherwave::WindowManager>(window, layoutEngine.get());

        // Ensure initial window dimensions are correct
        updateWindowDimensions();

        std::cout << "✅ SDL2 visual engine initialized successfully!" << std::endl;
        return true;
    }

    void cleanup() {
        // Free textures
        for (auto texture : imageTextures) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
        imageTextures.clear();

        // Destroy renderer and window
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }

        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        // Quit SDL subsystems
        IMG_Quit();
        SDL_Quit();

        std::cout << "🧹 SDL cleanup completed" << std::endl;
    }

    void loadImages() {
        std::cout << "📸 Loading images..." << std::endl;

        imagePaths.clear();

        // Free existing textures
        for (auto texture : imageTextures) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
        imageTextures.clear();

        // Use relative path from binary location (build/bin/)
        std::string assetsDir = "assets/images";
        if (!std::filesystem::exists(assetsDir)) {
            std::cout << "📁 No assets directory found at: " << assetsDir << std::endl;
            return;
        }

        // Scan for image files
        for (const auto& entry : std::filesystem::directory_iterator(assetsDir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" ||
                    ext == ".gif" || ext == ".bmp" || ext == ".tiff" || ext == ".tif") {

                    std::string imagePath = entry.path().string();
                    imagePaths.push_back(imagePath);

                    // Load image as texture
                    SDL_Surface* surface = IMG_Load(imagePath.c_str());
                    if (surface) {
                        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                        SDL_FreeSurface(surface);

                        if (texture) {
                            imageTextures.push_back(texture);
                            std::cout << "✅ Loaded: " << std::filesystem::path(imagePath).filename().string() << std::endl;
                        } else {
                            std::cerr << "❌ Failed to create texture from: " << imagePath << std::endl;
                            imageTextures.push_back(nullptr);
                        }
                    } else {
                        std::cerr << "❌ Failed to load image: " << imagePath << std::endl;
                        imageTextures.push_back(nullptr);
                    }
                }
            }
        }

        std::cout << "📸 Loaded " << imagePaths.size() << " images with " << imageTextures.size() << " textures" << std::endl;

        if (imagePaths.empty()) {
            std::cout << "💡 Add images to assets/images/ (project root) and restart" << std::endl;
        } else {
            // Force layout calculation after loading images to ensure proper initial sizing
            updateWindowDimensions();
            if (!imageTextures.empty()) {
                // Mark layout for initial calculation
                layoutNeedsRecalc = true;
                DEBUG_LAYOUT("FALLBACK_INIT", "Initial layout will be calculated on first render");
                std::cout << "🎨 Initial layout marked for calculation (using fallback)" << std::endl;
            }
        }
    }

    void render() {
        // Clear screen with theme background color
        SDL_Color bgColor = themeManager->getBackgroundColor();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderClear(renderer);

        // Render images with proper transitions
        if (!imagePaths.empty() && !imageTextures.empty()) {
            if (isTransitioning) {
                renderTransition();
            } else {
                renderCurrentImage();
            }
        } else if (imagePaths.empty()) {
            // Show "no images" message
            renderNoImagesMessage();
        }

        // Render UI overlay
        renderUI();

        // Present the frame
        SDL_RenderPresent(renderer);
    }

    void renderCurrentImage() {
        if (currentIndex >= imageTextures.size() || !imageTextures[currentIndex]) {
            DEBUG_LOG("RENDER", "renderCurrentImage() - no valid texture at index " + std::to_string(currentIndex));
            return;
        }

        SDL_Texture* currentTexture = imageTextures[currentIndex];

        // Use cached layout if available, or calculate and cache it
        SDL_Rect destRect;
        if (layoutNeedsRecalc || cachedImageIndex != currentIndex) {
            // Calculate layout and cache it
            DEBUG_LOG("LAYOUT_FALLBACK", "Calculating and caching layout for image " + std::to_string(currentIndex));
            destRect = calculateImageRectFallback(currentTexture);
            cachedImageRect = destRect;
            cachedImageIndex = currentIndex;
            layoutNeedsRecalc = false;

            DEBUG_LOG("FALLBACK_CACHED", "Cached layout: " + std::to_string(destRect.w) + "x" + std::to_string(destRect.h) +
                      " at (" + std::to_string(destRect.x) + "," + std::to_string(destRect.y) + ")");
        } else {
            // Use cached layout (no debug spam)
            destRect = cachedImageRect;
        }

        // Reset alpha to full opacity
        SDL_SetTextureAlphaMod(currentTexture, 255);
        SDL_RenderCopy(renderer, currentTexture, nullptr, &destRect);
    }

    void renderTransition() {
        // During transitions, we want to show:
        // - currentTexture: the image we're transitioning FROM (previous image)
        // - nextTexture: the image we're transitioning TO (current image)

        // Calculate the previous index (what we're transitioning FROM)
        size_t previousIndex = (currentIndex == 0) ? imagePaths.size() - 1 : currentIndex - 1;
        size_t nextIndex = currentIndex; // What we're transitioning TO

        // Ensure both textures are valid
        if (previousIndex >= imageTextures.size() || !imageTextures[previousIndex] ||
            nextIndex >= imageTextures.size() || !imageTextures[nextIndex]) {
            renderCurrentImage();
            return;
        }

        SDL_Texture* currentTexture = imageTextures[previousIndex]; // FROM
        SDL_Texture* nextTexture = imageTextures[nextIndex];        // TO

        // Use cached layout calculations for transitions to prevent recalculation during animation
        SDL_Rect currentRect, nextRect;

        // CRITICAL FIX: Only recalculate if we don't have cached values for these specific indices
        // or if the layout was marked for recalculation (window resize/move)
        bool needsRecalc = (cachedTransitionCurrentIndex != previousIndex ||
                           cachedTransitionNextIndex != nextIndex ||
                           layoutNeedsRecalc);

        if (needsRecalc) {
            // Calculate and cache both layouts
            currentRect = calculateImageRectFallback(currentTexture);
            nextRect = calculateImageRectFallback(nextTexture);

            cachedTransitionCurrentRect = currentRect;
            cachedTransitionNextRect = nextRect;
            cachedTransitionCurrentIndex = previousIndex;
            cachedTransitionNextIndex = nextIndex;

            DEBUG_LOG("TRANSITION_RECALC", "Recalculated transition layouts: from=" + std::to_string(previousIndex) +
                      " (" + std::to_string(currentRect.w) + "x" + std::to_string(currentRect.h) + ")" +
                      " to=" + std::to_string(nextIndex) +
                      " (" + std::to_string(nextRect.w) + "x" + std::to_string(nextRect.h) + ")");
        } else {
            // Use cached layouts (no debug spam during smooth transitions)
            currentRect = cachedTransitionCurrentRect;
            nextRect = cachedTransitionNextRect;
        }

        // Apply transition effects based on type
        switch (currentTransitionType) {
            case TransitionType::FADE:
                renderFadeTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
            case TransitionType::GLITCH:
                renderGlitchTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
            case TransitionType::SOFT_FADE:
                renderSoftFadeTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
            case TransitionType::PIXEL:
                renderPixelTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
            case TransitionType::DISSOLVE:
                renderDissolveTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
            default:
                renderFadeTransition(currentTexture, nextTexture, currentRect, nextRect);
                break;
        }
    }

    SDL_Rect calculateImageRectFallback(SDL_Texture* texture) {
        if (!texture) {
            DEBUG_LOG("FALLBACK", "ERROR: null texture provided");
            return {0, 0, 0, 0};
        }

        int textureWidth, textureHeight;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight) != 0) {
            DEBUG_LOG("FALLBACK", "ERROR: failed to query texture dimensions");
            return {0, 0, 0, 0};
        }

        if (textureWidth <= 0 || textureHeight <= 0 || windowWidth <= 0 || windowHeight <= 0) {
            DEBUG_LOG("FALLBACK", "ERROR: invalid dimensions - texture:" + std::to_string(textureWidth) + "x" +
                      std::to_string(textureHeight) + " window:" + std::to_string(windowWidth) + "x" + std::to_string(windowHeight));
            return {0, 0, 0, 0};
        }

        // Calculate aspect-ratio-preserving scaling with improved precision
        float windowAspect = (float)windowWidth / windowHeight;
        float textureAspect = (float)textureWidth / textureHeight;

        int scaledWidth, scaledHeight;

        if (textureAspect > windowAspect) {
            // Image is wider than window - fit to width
            scaledWidth = windowWidth;
            scaledHeight = (int)round(windowWidth / textureAspect);
        } else {
            // Image is taller than window - fit to height
            scaledHeight = windowHeight;
            scaledWidth = (int)round(windowHeight * textureAspect);
        }

        // Center the image
        int x = (windowWidth - scaledWidth) / 2;
        int y = (windowHeight - scaledHeight) / 2;

        return { x, y, scaledWidth, scaledHeight };
    }

    void renderFadeTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                             const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
        // Clear background first to prevent ghosting
        SDL_Color bgColor = themeManager->getBackgroundColor();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, nullptr);

        // Render current image with decreasing alpha
        SDL_SetTextureAlphaMod(currentTexture, (Uint8)((1.0f - fadeAlpha) * 255));
        SDL_RenderCopy(renderer, currentTexture, nullptr, &currentRect);

        // Render next image with increasing alpha
        SDL_SetTextureAlphaMod(nextTexture, (Uint8)(fadeAlpha * 255));
        SDL_RenderCopy(renderer, nextTexture, nullptr, &nextRect);
    }

    void renderSoftFadeTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                                 const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
        // Use eased fade for organic feel
        float easedAlpha = 0.5f * (1 + sin(M_PI * fadeAlpha - M_PI/2)); // Smoothstep

        // Add subtle warmth during transition
        SDL_SetRenderDrawColor(renderer, 20, 15, 10, (Uint8)(easedAlpha * 30));
        SDL_RenderFillRect(renderer, nullptr);

        // Render current image
        SDL_SetTextureAlphaMod(currentTexture, (Uint8)((1.0f - easedAlpha) * 255));
        SDL_RenderCopy(renderer, currentTexture, nullptr, &currentRect);

        // Render next image
        SDL_SetTextureAlphaMod(nextTexture, (Uint8)(easedAlpha * 255));
        SDL_RenderCopy(renderer, nextTexture, nullptr, &nextRect);
    }

    void renderGlitchTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                               const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
        float progress = fadeAlpha;

        // Render current image with glitch effects
        if (progress < 0.8f) {
            SDL_SetTextureAlphaMod(currentTexture, (Uint8)((1.0f - progress * 1.25f) * 255));

            // Apply chromatic aberration effect
            SDL_SetTextureColorMod(currentTexture, 255, (Uint8)(255 * (1 - glitchIntensity * 0.3f)), 255);

            // Render with random horizontal offsets for glitch effect
            if (glitchRng() % 10 < 3) { // 30% chance of glitch strip
                SDL_Rect glitchRect = currentRect;
                glitchRect.x += static_cast<int>((glitchRng() % 20) - 10); // Random offset
                SDL_RenderCopy(renderer, currentTexture, nullptr, &glitchRect);
            } else {
                SDL_RenderCopy(renderer, currentTexture, nullptr, &currentRect);
            }

            // Reset color modulation
            SDL_SetTextureColorMod(currentTexture, 255, 255, 255);
        }

        // Render next image
        SDL_SetTextureAlphaMod(nextTexture, (Uint8)(progress * 255));
        SDL_RenderCopy(renderer, nextTexture, nullptr, &nextRect);

        // Add glitch lines
        if (glitchRng() % 20 < 1) { // 5% chance
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 150); // Cyan glitch
            int y = static_cast<int>(glitchRng() % windowHeight);
            SDL_Rect glitchLine = {0, y, windowWidth, 2};
            SDL_RenderFillRect(renderer, &glitchLine);
        }

        // Add magenta noise blocks
        if (progress > 0.3f && glitchRng() % 30 < 1) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 100); // Magenta noise
            SDL_Rect noiseRect = {
                static_cast<int>(glitchRng() % windowWidth),
                static_cast<int>(glitchRng() % windowHeight),
                static_cast<int>(glitchRng() % 50 + 10),
                static_cast<int>(glitchRng() % 20 + 5)
            };
            SDL_RenderFillRect(renderer, &noiseRect);
        }
    }

    void renderPixelTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                              const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
        // Sharp digital transition with blocky reveal
        int blockSize = 8;

        for (int x = 0; x < windowWidth; x += blockSize) {
            for (int y = 0; y < windowHeight; y += blockSize) {
                // Deterministic pattern based on position
                float threshold = sin(x * 0.02f) * sin(y * 0.02f) * 0.5f + 0.5f;

                SDL_Rect blockRect = {x, y, blockSize, blockSize};

                if (fadeAlpha > threshold) {
                    // Show next image block
                    SDL_SetTextureAlphaMod(nextTexture, 255);
                    SDL_RenderCopy(renderer, nextTexture, &blockRect, &blockRect);
                } else {
                    // Show current image block
                    SDL_SetTextureAlphaMod(currentTexture, 255);
                    SDL_RenderCopy(renderer, currentTexture, &blockRect, &blockRect);
                }
            }
        }
    }

    void renderDissolveTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                                 const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
        // Organic dissolve with noise pattern
        int pixelSize = 4;

        for (int x = 0; x < windowWidth; x += pixelSize) {
            for (int y = 0; y < windowHeight; y += pixelSize) {
                // Use noise for dissolve pattern
                float noiseValue = sin(x * 0.01f + y * 0.01f) * 0.5f + 0.5f;

                SDL_Rect pixelRect = {x, y, pixelSize, pixelSize};

                if (fadeAlpha > noiseValue) {
                    SDL_SetTextureAlphaMod(nextTexture, 255);
                    SDL_RenderCopy(renderer, nextTexture, &pixelRect, &pixelRect);
                } else {
                    SDL_SetTextureAlphaMod(currentTexture, 255);
                    SDL_RenderCopy(renderer, currentTexture, &pixelRect, &pixelRect);
                }
            }
        }
    }

    void renderNoImagesMessage() {
        // Simple text rendering (we'll improve this with SDL_ttf later)
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

        // Draw a simple rectangle as placeholder
        SDL_Rect messageRect = {
            windowWidth/4,
            windowHeight/2 - 100,
            windowWidth/2,
            200
        };
        SDL_RenderFillRect(renderer, &messageRect);

        // Border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &messageRect);
    }

    void renderUI() {
        if (imagePaths.empty()) return;

        // Minimal UI - only show debug overlays when explicitly enabled
        // Remove the persistent info bar that was causing visual artifacts

        // Theme debug overlay
        if (showThemeDebug) {
            renderThemeDebug();
        }

        // Layout debug overlay
        if (showLayoutDebug) {
            renderLayoutDebug();
        }
    }

    void renderThemeDebug() {
        // Theme debug panel in top-right
        int panelWidth = 300;
        int panelHeight = 200;
        SDL_Rect debugRect = {
            windowWidth - panelWidth - 10,
            10,
            panelWidth,
            panelHeight
        };

        // Semi-transparent background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &debugRect);

        // Border with theme accent color
        SDL_Color accentColor = themeManager->getAccentColor();
        SDL_SetRenderDrawColor(renderer, accentColor.r, accentColor.g, accentColor.b, 255);
        SDL_RenderDrawRect(renderer, &debugRect);

        // Color swatches for primary colors
        auto theme = themeManager->getCurrentTheme();
        int swatchSize = 30;
        int x = debugRect.x + 10;
        int y = debugRect.y + 40;

        // Primary color swatch
        SDL_Color primaryColor = themeManager->getPrimaryColor();
        SDL_Rect primarySwatch = { x, y, swatchSize, swatchSize };
        SDL_SetRenderDrawColor(renderer, primaryColor.r, primaryColor.g, primaryColor.b, 255);
        SDL_RenderFillRect(renderer, &primarySwatch);

        // Accent color swatch
        SDL_Rect accentSwatch = { x + swatchSize + 5, y, swatchSize, swatchSize };
        SDL_SetRenderDrawColor(renderer, accentColor.r, accentColor.g, accentColor.b, 255);
        SDL_RenderFillRect(renderer, &accentSwatch);

        // Background color swatch
        SDL_Color bgColor = themeManager->getBackgroundColor();
        SDL_Rect bgSwatch = { x + (swatchSize + 5) * 2, y, swatchSize, swatchSize };
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, &bgSwatch);

        // White borders for swatches
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &primarySwatch);
        SDL_RenderDrawRect(renderer, &accentSwatch);
        SDL_RenderDrawRect(renderer, &bgSwatch);
    }

    void renderLayoutDebug() {
        // Layout debug panel in top-left
        int panelWidth = 350;
        int panelHeight = 250;
        SDL_Rect debugRect = {
            10,
            10,
            panelWidth,
            panelHeight
        };

        // Semi-transparent background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &debugRect);

        // Border with accent color
        SDL_Color accentColor = themeManager->getAccentColor();
        SDL_SetRenderDrawColor(renderer, accentColor.r, accentColor.g, accentColor.b, 255);
        SDL_RenderDrawRect(renderer, &debugRect);

        // Visualize current layout rectangles
        const auto& layouts = layoutEngine->getImageLayouts();

        // Draw layout rectangles with different colors
        SDL_Color layoutColors[] = {
            {255, 100, 100, 100}, // Red
            {100, 255, 100, 100}, // Green
            {100, 100, 255, 100}, // Blue
            {255, 255, 100, 100}, // Yellow
            {255, 100, 255, 100}, // Magenta
            {100, 255, 255, 100}  // Cyan
        };

        for (size_t i = 0; i < layouts.size() && i < 6; i++) {
            SDL_Rect layoutRect = layoutEngine->getImageRect(i);
            if (layoutRect.w > 0 && layoutRect.h > 0) {
                SDL_Color& color = layoutColors[i];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &layoutRect);

                // Draw border
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                SDL_RenderDrawRect(renderer, &layoutRect);
            }
        }

        // Reset draw color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

    void handleInput() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        case SDLK_q:
                            isRunning = false;
                            break;

                        case SDLK_SPACE:
                        case SDLK_RIGHT:
                        case SDLK_n:
                            DEBUG_LOG("INPUT", "Next image key pressed (SPACE/RIGHT/N)");
                            nextImage();
                            break;

                        case SDLK_LEFT:
                        case SDLK_BACKSPACE:
                        case SDLK_p:
                            DEBUG_LOG("INPUT", "Previous image key pressed (LEFT/BACKSPACE/P)");
                            previousImage();
                            break;

                        case SDLK_r:
                            loadImages();
                            break;

                        case SDLK_f:
                            toggleFullscreen();
                            break;

                        case SDLK_i:
                            showInfo();
                            break;

                        case SDLK_t:
                            toggleThemeDebug();
                            break;

                        case SDLK_u:
                            refreshTheme();
                            break;

                        case SDLK_l:
                            toggleLayoutDebug();
                            break;

                        case SDLK_1:
                            layoutEngine->setLayoutMode(LayoutMode::SINGLE_FULLSCREEN);
                            break;

                        case SDLK_2:
                            layoutEngine->setLayoutMode(LayoutMode::DUAL_SPLIT);
                            break;

                        case SDLK_3:
                            layoutEngine->setLayoutMode(LayoutMode::GALLERY_MOSAIC);
                            break;

                        case SDLK_a:
                            layoutEngine->setLayoutMode(LayoutMode::ADAPTIVE_AUTO);
                            break;
                    }
                    break;

                case SDL_WINDOWEVENT:
                    std::cout << "🪟 Window event: " << e.window.event << std::endl;
                    DEBUG_WINDOW("SDL_EVENT", "Window event type: " + std::to_string(e.window.event));

                    windowManager->handleWindowEvent(e.window);
                    updateWindowDimensions(); // Always update dimensions on window events
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        std::cout << "🔄 Window resized to: " << windowWidth << "x" << windowHeight << std::endl;
                        DEBUG_WINDOW("RESIZE", "New size: " + std::to_string(windowWidth) + "x" + std::to_string(windowHeight));
                    } else if (e.window.event == SDL_WINDOWEVENT_MOVED) {
                        int x, y;
                        SDL_GetWindowPosition(window, &x, &y);
                        DEBUG_WINDOW("MOVE_EVENT", "Window moved to position (" + std::to_string(x) + "," + std::to_string(y) + ")");

                        // Check if window moved to a different display
                        int newDisplayIndex = SDL_GetWindowDisplayIndex(window);
                        static int lastDisplayIndex = 0; // Track display changes

                        if (newDisplayIndex != lastDisplayIndex) {
                            DEBUG_DISPLAY("DISPLAY_CHANGE", "Window moved from display " +
                                         std::to_string(lastDisplayIndex) + " to display " +
                                         std::to_string(newDisplayIndex));

                            // Get new display bounds for proper layout calculation
                            SDL_Rect displayBounds;
                            if (SDL_GetDisplayBounds(newDisplayIndex, &displayBounds) == 0) {
                                DEBUG_DISPLAY("NEW_DISPLAY_BOUNDS", "Display " + std::to_string(newDisplayIndex) +
                                             " bounds: " + std::to_string(displayBounds.w) + "x" +
                                             std::to_string(displayBounds.h));
                            }
                            lastDisplayIndex = newDisplayIndex;
                        }

                        // Force layout recalculation on window move to detect monitor changes
                        if (!imageTextures.empty()) {
                            layoutNeedsRecalc = true;

                            // Invalidate all caches for multi-monitor layout changes
                            cachedImageIndex = SIZE_MAX;
                            cachedTransitionCurrentIndex = SIZE_MAX;
                            cachedTransitionNextIndex = SIZE_MAX;

                            DEBUG_LAYOUT("RECALC_TRIGGER", "All layout caches invalidated due to window move");
                        }
                    }
                    break;
            }
        }
    }

    void updateWindowDimensions() {
        // Get current window size to ensure windowWidth/windowHeight are always accurate
        int newWidth, newHeight;
        SDL_GetWindowSize(window, &newWidth, &newHeight);

        // Only update if dimensions have actually changed
        if (newWidth != windowWidth || newHeight != windowHeight) {
            windowWidth = newWidth;
            windowHeight = newHeight;

            // Mark layout for recalculation since window size changed
            layoutNeedsRecalc = true;

            // Also invalidate all caches since window dimensions changed
            cachedImageIndex = SIZE_MAX;
            cachedTransitionCurrentIndex = SIZE_MAX;
            cachedTransitionNextIndex = SIZE_MAX;

            DEBUG_LOG("WINDOW_RESIZE", "Window dimensions changed to " + std::to_string(windowWidth) + "x" + std::to_string(windowHeight) + " - all caches invalidated");

            // Also update the layout engine with current dimensions
            if (layoutEngine) {
                layoutEngine->updateWindowDimensions(windowWidth, windowHeight);
            }
        }
    }

    void nextImage() {
        if (imagePaths.empty()) {
            DEBUG_LOG("IMAGE_SWITCH", "nextImage() called but no images available");
            return;
        }

        // Allow interrupting slow transitions for faster navigation
        if (isTransitioning) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - transitionStart).count();

            // If transition is more than 50% complete or been running for more than 200ms, allow interrupt
            if (fadeAlpha > 0.5f || elapsed > 200) {
                isTransitioning = false;
                fadeAlpha = 1.0f;
                DEBUG_LOG("IMAGE_SWITCH", "Interrupted transition for faster navigation");
            } else {
                DEBUG_LOG("IMAGE_SWITCH", "nextImage() blocked - already transitioning");
                return;
            }
        }

        // Update index BEFORE starting transition and calculating layout
        size_t previousIndex = currentIndex;
        currentIndex = (currentIndex + 1) % imagePaths.size();

        DEBUG_LOG("IMAGE_SWITCH", "nextImage() from index " + std::to_string(previousIndex) +
                  " to " + std::to_string(currentIndex));

        // Mark layout for recalculation since image changed
        layoutNeedsRecalc = true;

        // CRITICAL FIX: Don't invalidate transition cache here - let renderTransition() handle it
        // The transition cache will be recalculated only if the indices don't match

        startTransition();

        std::string filename = std::filesystem::path(imagePaths[currentIndex]).filename().string();
        std::cout << "➡️ Next: " << filename << " (" << (currentIndex + 1) << "/" << imagePaths.size() << ")" << std::endl;
    }

    void previousImage() {
        if (imagePaths.empty()) {
            DEBUG_LOG("IMAGE_SWITCH", "previousImage() called but no images available");
            return;
        }

        // Allow interrupting slow transitions for faster navigation
        if (isTransitioning) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - transitionStart).count();

            // If transition is more than 50% complete or been running for more than 200ms, allow interrupt
            if (fadeAlpha > 0.5f || elapsed > 200) {
                isTransitioning = false;
                fadeAlpha = 1.0f;
                DEBUG_LOG("IMAGE_SWITCH", "Interrupted transition for faster navigation");
            } else {
                DEBUG_LOG("IMAGE_SWITCH", "previousImage() blocked - already transitioning");
                return;
            }
        }

        // Move to previous index first, then start transition
        size_t previousIndex = currentIndex;
        currentIndex = (currentIndex == 0) ? imagePaths.size() - 1 : currentIndex - 1;

        DEBUG_LOG("IMAGE_SWITCH", "previousImage() from index " + std::to_string(previousIndex) +
                  " to " + std::to_string(currentIndex));

        // Mark layout for recalculation since image changed
        layoutNeedsRecalc = true;

        // CRITICAL FIX: Don't invalidate transition cache here - let renderTransition() handle it
        // The transition cache will be recalculated only if the indices don't match

        startTransition();

        std::string filename = std::filesystem::path(imagePaths[currentIndex]).filename().string();
        std::cout << "⬅️ Previous: " << filename << " (" << (currentIndex + 1) << "/" << imagePaths.size() << ")" << std::endl;
    }

    void startTransition() {
        isTransitioning = true;
        transitionStart = std::chrono::steady_clock::now();
        fadeAlpha = 0.0f;

        // Set transition type based on current theme
        std::string themeTransition = themeManager->getTransitionType();
        if (themeTransition == "glitch") {
            currentTransitionType = TransitionType::GLITCH;
            glitchIntensity = themeManager->getEffectIntensity();
        } else if (themeTransition == "soft_fade") {
            currentTransitionType = TransitionType::SOFT_FADE;
        } else if (themeTransition == "pixel") {
            currentTransitionType = TransitionType::PIXEL;
        } else if (themeTransition == "dissolve") {
            currentTransitionType = TransitionType::DISSOLVE;
        } else {
            currentTransitionType = TransitionType::FADE;
        }

        // PERFORMANCE FIX: Reset layoutNeedsRecalc to false after transition starts
        // This prevents renderTransition() from recalculating every frame
        // The transition cache logic will handle the initial calculation
        layoutNeedsRecalc = false;
    }

    void updateTransition() {
        if (!isTransitioning) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - transitionStart).count();

        // Use theme-specific transition duration, but cap it for responsiveness
        float transitionDuration = std::min(themeManager->getTransitionDuration() * 1000.0f, 500.0f);
        float progress = elapsed / transitionDuration;

        if (progress >= 1.0f) {
            fadeAlpha = 1.0f;
            isTransitioning = false;
            glitchIntensity = 0.0f;

            // DO NOT update currentIndex here - it was already updated in nextImage()/previousImage()
            // The index jumping bug was caused by double-incrementing the index
        } else {
            // Apply different easing based on transition type
            switch (currentTransitionType) {
                case TransitionType::SOFT_FADE:
                    // Ease-in-out for organic feel
                    fadeAlpha = 0.5f * (1 + sin(M_PI * progress - M_PI/2));
                    break;
                case TransitionType::GLITCH:
                    // Quick snap with glitch effects
                    fadeAlpha = progress > 0.7f ? 1.0f : progress * 1.4f;
                    glitchIntensity = sin(progress * M_PI) * themeManager->getEffectIntensity();
                    break;
                case TransitionType::PIXEL:
                    // Sharp digital transition
                    fadeAlpha = progress > 0.5f ? 1.0f : 0.0f;
                    break;
                default:
                    fadeAlpha = progress;
                    break;
            }
        }
    }

    void applyGlitchEffect(SDL_Texture* texture, int x, int y, int w, int h) {
        if (currentTransitionType != TransitionType::GLITCH || glitchIntensity <= 0.0f) {
            return;
        }

        // Simple glitch effect with random offset strips
        std::uniform_int_distribution<int> offsetDist(-10, 10);
        std::uniform_int_distribution<int> heightDist(5, 20);

        for (int i = 0; i < 5; ++i) {
            int stripY = glitchRng() % h;
            int stripHeight = heightDist(glitchRng);
            int offsetX = offsetDist(glitchRng) * glitchIntensity;

            SDL_Rect srcRect = {0, stripY, w, stripHeight};
            SDL_Rect dstRect = {x + offsetX, y + stripY, w, stripHeight};

            // Render with color modulation for chromatic aberration
            SDL_SetTextureColorMod(texture, 255, (Uint8)(255 * (1 - glitchIntensity * 0.3)), 255);
            SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
        }

        // Reset color modulation
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    }

    void toggleFullscreen() {
        isFullscreen = !isFullscreen;

        if (isFullscreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            std::cout << "🖥️ Fullscreen mode enabled" << std::endl;
        } else {
            SDL_SetWindowFullscreen(window, 0);
            std::cout << "🪟 Windowed mode enabled" << std::endl;
        }
    }

    void refreshTheme() {
        std::cout << "🎨 Refreshing theme from content analysis..." << std::endl;
        if (themeManager->loadThemeFromAPI(imagePaths)) {
            std::cout << "✅ Theme updated successfully" << std::endl;
            themeManager->printThemeInfo();
        } else {
            std::cout << "⚠️ Using fallback theme" << std::endl;
        }
    }

    void toggleThemeDebug() {
        showThemeDebug = !showThemeDebug;
        if (showThemeDebug) {
            std::cout << "🎨 Theme debug overlay enabled" << std::endl;
            themeManager->printThemeInfo();
        } else {
            std::cout << "🎨 Theme debug overlay disabled" << std::endl;
        }
    }

    void toggleLayoutDebug() {
        showLayoutDebug = !showLayoutDebug;
        if (showLayoutDebug) {
            std::cout << "🎨 Layout debug overlay enabled" << std::endl;
            layoutEngine->printLayoutInfo();
            layoutEngine->printDisplayInfo();
        } else {
            std::cout << "🎨 Layout debug overlay disabled" << std::endl;
        }
    }

    void showInfo() {
        std::cout << "\n📊 Aetherwave Display Engine Status:" << std::endl;
        std::cout << "   Version: 2.0.0 (SDL2)" << std::endl;
        std::cout << "   Window: " << windowWidth << "x" << windowHeight << std::endl;
        std::cout << "   Images loaded: " << imagePaths.size() << std::endl;
        std::cout << "   Current index: " << (imagePaths.empty() ? 0 : currentIndex + 1) << std::endl;
        std::cout << "   Fullscreen: " << (isFullscreen ? "Yes" : "No") << std::endl;

        if (!imagePaths.empty()) {
            std::cout << "   Current image: " << std::filesystem::path(imagePaths[currentIndex]).filename().string() << std::endl;
        }
        std::cout << std::endl;
    }

    void run() {
        std::cout << "\n🌊 ==================================" << std::endl;
        std::cout << "    AETHERWAVE VISUAL DISPLAY ENGINE" << std::endl;
        std::cout << "    Real-time SDL2 Implementation" << std::endl;
        std::cout << "    ==================================" << std::endl;

        if (!initialize()) {
            std::cerr << "❌ Failed to initialize SDL2 engine" << std::endl;
            return;
        }

        loadImages();

        std::cout << "\n🎮 Controls:" << std::endl;
        std::cout << "   [SPACE/→/N] Next image    [←/BACKSPACE/P] Previous image" << std::endl;
        std::cout << "   [F] Toggle fullscreen    [R] Reload images" << std::endl;
        std::cout << "   [I] Show info            [T] Theme debug" << std::endl;
        std::cout << "   [U] Update theme         [L] Layout debug" << std::endl;
        std::cout << "   [1] Single mode          [2] Dual split" << std::endl;
        std::cout << "   [3] Gallery mosaic       [A] Adaptive mode" << std::endl;
        std::cout << "   [ESC/Q] Quit" << std::endl;

        // Load initial theme from content
        std::cout << "\n🎨 Analyzing content for theme..." << std::endl;
        refreshTheme();

        std::cout << "\n🚀 Visual display engine starting..." << std::endl;

        // Main render loop
        auto lastTime = std::chrono::steady_clock::now();
        while (isRunning) {
            auto currentTime = std::chrono::steady_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

            // Target 60 FPS (16.67ms per frame)
            if (deltaTime >= 16) {
                handleInput();
                updateTransition();
                render();
                lastTime = currentTime;
            } else {
                // Sleep for a short time to prevent busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        cleanup();
        std::cout << "\n👋 Aetherwave display engine shutdown complete" << std::endl;
    }
};

} // namespace Aetherwave

int main() {
    std::cout << "🌊 Aetherwave Visual Display Engine v2.0" << std::endl;
    std::cout << "📱 Real Graphics Mode (SDL2)" << std::endl;

    try {
        Aetherwave::VisualDisplayEngine engine;
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
