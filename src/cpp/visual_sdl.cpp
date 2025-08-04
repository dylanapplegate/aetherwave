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
            if (layoutEngine && !imageTextures.empty()) {
                std::vector<SDL_Texture*> textures = { imageTextures[0] };
                layoutEngine->calculateLayout(textures);
                std::cout << "🎨 Initial layout calculated for first image" << std::endl;
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
        if (currentIndex >= imageTextures.size() || !imageTextures[currentIndex]) return;

        SDL_Texture* currentTexture = imageTextures[currentIndex];

        // Use pre-calculated layout from layout engine (no recalculation during rendering)
        SDL_Rect destRect = layoutEngine->getImageRect(0);
        if (destRect.w == 0 || destRect.h == 0) {
            // Fallback if layout engine fails
            std::cout << "⚠️ Layout engine returned invalid rect, using fallback calculation" << std::endl;
            destRect = calculateImageRectFallback(currentTexture);
        }

        // Debug: Print current layout calculations
        int textureW, textureH;
        SDL_QueryTexture(currentTexture, nullptr, nullptr, &textureW, &textureH);
        std::cout << "🖼️ RENDER DEBUG: texture=" << textureW << "x" << textureH
                  << " window=" << windowWidth << "x" << windowHeight
                  << " destRect=" << destRect.w << "x" << destRect.h
                  << " at (" << destRect.x << "," << destRect.y << ")" << std::endl;

        // Reset alpha to full opacity
        SDL_SetTextureAlphaMod(currentTexture, 255);
        SDL_RenderCopy(renderer, currentTexture, nullptr, &destRect);
    }

    void renderTransition() {
        size_t nextIndex = (currentIndex + 1) % imageTextures.size();

        // Ensure both textures are valid
        if (currentIndex >= imageTextures.size() || !imageTextures[currentIndex] ||
            nextIndex >= imageTextures.size() || !imageTextures[nextIndex]) {
            renderCurrentImage();
            return;
        }

        SDL_Texture* currentTexture = imageTextures[currentIndex];
        SDL_Texture* nextTexture = imageTextures[nextIndex];

        // Use pre-calculated layout from layout engine (no recalculation during rendering)
        SDL_Rect currentRect = layoutEngine->getImageRect(0);
        SDL_Rect nextRect = layoutEngine->getImageRect(0); // Use same layout for transition

        // Fallback to old calculation if layout engine fails
        if (currentRect.w == 0 || currentRect.h == 0) {
            currentRect = calculateImageRectFallback(currentTexture);
        }
        if (nextRect.w == 0 || nextRect.h == 0) {
            nextRect = calculateImageRectFallback(nextTexture);
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
        if (!texture) return {0, 0, 0, 0};

        int textureWidth, textureHeight;
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);

        std::cout << "🔧 Fallback calculation: window=" << windowWidth << "x" << windowHeight
                  << " texture=" << textureWidth << "x" << textureHeight << std::endl;

        // Calculate aspect-ratio-preserving scaling
        float scaleX = (float)windowWidth / textureWidth;
        float scaleY = (float)windowHeight / textureHeight;
        float scale = std::min(scaleX, scaleY);

        int scaledWidth = (int)(textureWidth * scale);
        int scaledHeight = (int)(textureHeight * scale);

        // Center the image
        int x = (windowWidth - scaledWidth) / 2;
        int y = (windowHeight - scaledHeight) / 2;

        std::cout << "🔧 Fallback result: " << scaledWidth << "x" << scaledHeight
                  << " at (" << x << "," << y << ")" << std::endl;

        return { x, y, scaledWidth, scaledHeight };
    }

    void renderFadeTransition(SDL_Texture* currentTexture, SDL_Texture* nextTexture,
                             const SDL_Rect& currentRect, const SDL_Rect& nextRect) {
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

        // Simple UI overlay showing current image info
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_Rect uiRect = { 10, windowHeight - 60, 400, 50 };
        SDL_RenderFillRect(renderer, &uiRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &uiRect);

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
                            nextImage();
                            break;

                        case SDLK_LEFT:
                        case SDLK_p:
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
                        std::cout << "🖥️ Window moved, forcing layout recalculation" << std::endl;
                        DEBUG_WINDOW("MOVE_EVENT", "Window moved to position (" + std::to_string(x) + "," + std::to_string(y) + ")");

                        // Force layout recalculation on window move to detect monitor changes
                        if (layoutEngine && !imageTextures.empty()) {
                            std::vector<SDL_Texture*> textures = { imageTextures[currentIndex] };
                            DEBUG_LAYOUT("RECALC_TRIGGER", "Layout recalculation triggered by window move");
                            layoutEngine->calculateLayout(textures);
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

            // Also update the layout engine with current dimensions
            if (layoutEngine) {
                layoutEngine->updateWindowDimensions(windowWidth, windowHeight);
            }
        }
    }

    void nextImage() {
        if (imagePaths.empty()) return;

        // Don't start new transition if already transitioning
        if (isTransitioning) return;

        startTransition();

        // Calculate layout for the new image once when switching
        if (layoutEngine && currentIndex < imageTextures.size() && imageTextures[currentIndex]) {
            std::vector<SDL_Texture*> textures = { imageTextures[currentIndex] };
            layoutEngine->calculateLayout(textures);
            std::cout << "🎨 Layout calculated for new image" << std::endl;
        }

        std::string filename = std::filesystem::path(imagePaths[currentIndex]).filename().string();
        std::cout << "➡️ Next: " << filename << " (" << (currentIndex + 1) << "/" << imagePaths.size() << ")" << std::endl;
    }

    void previousImage() {
        if (imagePaths.empty()) return;

        // Don't start new transition if already transitioning
        if (isTransitioning) return;

        // Move to previous index first, then start transition
        currentIndex = (currentIndex == 0) ? imagePaths.size() - 1 : currentIndex - 1;
        startTransition();

        // Calculate layout for the new image once when switching
        if (layoutEngine && currentIndex < imageTextures.size() && imageTextures[currentIndex]) {
            std::vector<SDL_Texture*> textures = { imageTextures[currentIndex] };
            layoutEngine->calculateLayout(textures);
            std::cout << "🎨 Layout calculated for new image" << std::endl;
        }

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
    }

    void updateTransition() {
        if (!isTransitioning) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - transitionStart).count();

        // Use theme-specific transition duration
        float transitionDuration = themeManager->getTransitionDuration() * 1000.0f;
        float progress = elapsed / transitionDuration;

        if (progress >= 1.0f) {
            fadeAlpha = 1.0f;
            isTransitioning = false;
            glitchIntensity = 0.0f;

            // Update to next image after transition completes
            currentIndex = (currentIndex + 1) % imagePaths.size();
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
        std::cout << "   [SPACE/→/N] Next image    [←/P] Previous image" << std::endl;
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
