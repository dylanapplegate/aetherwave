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

namespace Aetherwave {

class VisualDisplayEngine {
private:
    // SDL Components
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Theme Management
    std::unique_ptr<Aetherwave::ThemeManagerSDL> themeManager;
    bool showThemeDebug = false;

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

        // Initialize random number generator
        glitchRng.seed(rd());

        // Initialize theme manager
        themeManager = std::make_unique<Aetherwave::ThemeManagerSDL>();

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "❌ SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }

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

        // Set blend mode for alpha transparency
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
        }
    }

    void render() {
        // Clear screen with theme background color
        SDL_Color bgColor = themeManager->getBackgroundColor();
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderClear(renderer);

        // Render current image if available
        if (!imagePaths.empty() && currentIndex < imageTextures.size() && imageTextures[currentIndex]) {
            SDL_Texture* currentTexture = imageTextures[currentIndex];

            // Get texture dimensions
            int textureWidth, textureHeight;
            SDL_QueryTexture(currentTexture, nullptr, nullptr, &textureWidth, &textureHeight);

            // Calculate aspect-ratio-preserving scaling
            float scaleX = (float)windowWidth / textureWidth;
            float scaleY = (float)windowHeight / textureHeight;
            float scale = std::min(scaleX, scaleY);

            int scaledWidth = (int)(textureWidth * scale);
            int scaledHeight = (int)(textureHeight * scale);

            // Center the image
            int x = (windowWidth - scaledWidth) / 2;
            int y = (windowHeight - scaledHeight) / 2;

            SDL_Rect destRect = { x, y, scaledWidth, scaledHeight };

            // Apply fade effect during transitions
            if (isTransitioning) {
                SDL_SetTextureAlphaMod(currentTexture, (Uint8)(fadeAlpha * 255));
            } else {
                SDL_SetTextureAlphaMod(currentTexture, 255);
            }

            // Render the image
            SDL_RenderCopy(renderer, currentTexture, nullptr, &destRect);
        } else if (imagePaths.empty()) {
            // Show "no images" message
            renderNoImagesMessage();
        }

        // Render UI overlay
        renderUI();

        // Present the frame
        SDL_RenderPresent(renderer);
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
                    }
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        windowWidth = e.window.data1;
                        windowHeight = e.window.data2;
                        std::cout << "🔄 Window resized to: " << windowWidth << "x" << windowHeight << std::endl;
                    }
                    break;
            }
        }
    }

    void nextImage() {
        if (imagePaths.empty()) return;

        startTransition();
        currentIndex = (currentIndex + 1) % imagePaths.size();

        std::string filename = std::filesystem::path(imagePaths[currentIndex]).filename().string();
        std::cout << "➡️ Next: " << filename << " (" << (currentIndex + 1) << "/" << imagePaths.size() << ")" << std::endl;
    }

    void previousImage() {
        if (imagePaths.empty()) return;

        startTransition();
        currentIndex = (currentIndex == 0) ? imagePaths.size() - 1 : currentIndex - 1;

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
        std::cout << "   [U] Update theme         [ESC/Q] Quit" << std::endl;

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
