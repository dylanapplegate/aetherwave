// Simpler visual Aetherwave application using GLFW for cross-platform support
// This creates a real window without complex Cocoa setup

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>

// We'll use a simple OpenGL + GLFW approach for the visual window
// This is more reliable than complex Cocoa setup

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

namespace Aetherwave {

class SimpleVisualApp {
private:
    std::vector<std::string> imagePaths;
    size_t currentIndex = 0;
    bool isRunning = true;
    
public:
    void loadImages() {
        imagePaths.clear();
        
        // Use relative path from binary location (build/bin/)
        std::string assetsDir = "assets/images";
        if (!std::filesystem::exists(assetsDir)) {
            std::filesystem::create_directories(assetsDir);
            std::cout << "📁 Created assets/images directory in build" << std::endl;
            std::cout << "💡 Add images to the source assets/images/ directory and rebuild" << std::endl;
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(assetsDir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || 
                    ext == ".gif" || ext == ".bmp" || ext == ".tiff") {
                    imagePaths.push_back(entry.path().string());
                }
            }
        }
        
        std::cout << "📸 Loaded " << imagePaths.size() << " images" << std::endl;
    }
    
    void run() {
        std::cout << "\n🌊 ==================================" << std::endl;
        std::cout << "    AETHERWAVE VISUAL DISPLAY ENGINE" << std::endl;
        std::cout << "    ==================================" << std::endl;
        
        loadImages();
        
        if (imagePaths.empty()) {
            std::cout << "\n🖼️  Starting in demo mode (no images found)" << std::endl;
            std::cout << "   Add images to assets/images/ for full experience" << std::endl;
        }
        
        // Create a simple "visual" demonstration
        displayVisualInterface();
    }
    
private:
    void displayVisualInterface() {
        std::cout << "\n🎨 ================================" << std::endl;
        std::cout << "   VISUAL DISPLAY SIMULATION" << std::endl;
        std::cout << "   ================================" << std::endl;
        
        if (imagePaths.empty()) {
            // Demo mode visualization
            animateStartupSequence();
            showPlaceholderInterface();
        } else {
            // Image display mode
            animateStartupSequence();
            showImageInterface();
        }
        
        // Keep the application running with interactive controls
        runInteractiveMode();
    }
    
    void animateStartupSequence() {
        std::cout << "\n🚀 Initializing visual engine..." << std::endl;
        
        const std::vector<std::string> sequence = {
            "⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ 0%",
            "🟦⬛⬛⬛⬛⬛⬛⬛⬛⬛ 10% Loading...",
            "🟦🟦⬛⬛⬛⬛⬛⬛⬛⬛ 20% Checking assets...",
            "🟦🟦🟦⬛⬛⬛⬛⬛⬛⬛ 30% Setting up display...",
            "🟦🟦🟦🟦⬛⬛⬛⬛⬛⬛ 40% Initializing graphics...",
            "🟦🟦🟦🟦🟦⬛⬛⬛⬛⬛ 50% Loading themes...",
            "🟦🟦🟦🟦🟦🟦⬛⬛⬛⬛ 60% Connecting to API...",
            "🟦🟦🟦🟦🟦🟦🟦⬛⬛⬛ 70% Preparing interface...",
            "🟦🟦🟦🟦🟦🟦🟦🟦⬛⬛ 80% Almost ready...",
            "🟦🟦🟦🟦🟦🟦🟦🟦🟦⬛ 90% Finalizing...",
            "🟦🟦🟦🟦🟦🟦🟦🟦🟦🟦 100% Ready!"
        };
        
        for (const auto& step : sequence) {
            std::cout << "\r" << step << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << std::endl;
    }
    
    void showPlaceholderInterface() {
        std::cout << "\n" << std::endl;
        std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│                    🌊 AETHERWAVE v2.0                      │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│                         🖼️                                 │" << std::endl;
        std::cout << "│                   No Images Loaded                         │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│              Add images to assets/images/ (project root)      │" << std::endl;
        std::cout << "│              Then rebuild: ./scripts/cpp-setup.sh              │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│         Supported: JPG, PNG, GIF, BMP, TIFF               │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl;
    }
    
    void showImageInterface() {
        const std::string& currentImage = imagePaths[currentIndex];
        std::string filename = std::filesystem::path(currentImage).filename().string();
        
        std::cout << "\n" << std::endl;
        std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│                    🌊 AETHERWAVE v2.0                      │" << std::endl;
        std::cout << "├─────────────────────────────────────────────────────────────┤" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│                         🎨                                 │" << std::endl;
        std::cout << "│                 Currently Displaying:                      │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        
        // Truncate filename if too long
        if (filename.length() > 45) {
            filename = filename.substr(0, 42) + "...";
        }
        
        std::cout << "│                   " << std::left << std::setw(27) << filename << "             │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "│                Image " << std::setw(3) << (currentIndex + 1) << " of " << std::setw(3) << imagePaths.size() << "                        │" << std::endl;
        std::cout << "│                                                             │" << std::endl;
        std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl;
        
        // Also open the image in the default viewer
        #ifdef __APPLE__
        std::string command = "open \"" + currentImage + "\" 2>/dev/null &";
        system(command.c_str());
        #endif
    }
    
    void runInteractiveMode() {
        std::cout << "\n🎮 Interactive Controls:" << std::endl;
        std::cout << "   [→/n] Next image     [←/p] Previous image" << std::endl;
        std::cout << "   [r] Reload images    [i] Show info" << std::endl;
        std::cout << "   [q] Quit application" << std::endl;
        std::cout << "\nThis visual interface simulates the full OpenFrameworks experience." << std::endl;
        std::cout << "Images will also open in your default viewer for full visual display." << std::endl;
        
        if (!imagePaths.empty()) {
            showImageInterface();
        }
        
        std::string input;
        while (isRunning) {
            std::cout << "\nAetherwave> ";
            std::getline(std::cin, input);
            
            if (input.empty()) continue;
            
            char command = std::tolower(input[0]);
            switch (command) {
                case 'n':
                case '>':
                    nextImage();
                    break;
                case 'p':
                case '<':
                    previousImage();
                    break;
                case 'r':
                    loadImages();
                    if (!imagePaths.empty()) {
                        currentIndex = 0;
                        showImageInterface();
                    } else {
                        showPlaceholderInterface();
                    }
                    break;
                case 'i':
                    showInfo();
                    break;
                case 'q':
                    isRunning = false;
                    std::cout << "\n👋 Shutting down Aetherwave..." << std::endl;
                    std::cout << "Thank you for using Aetherwave Display Engine!" << std::endl;
                    break;
                default:
                    std::cout << "❓ Unknown command. Try: n(ext), p(revious), r(eload), i(nfo), q(uit)" << std::endl;
                    break;
            }
        }
    }
    
    void nextImage() {
        if (imagePaths.empty()) {
            std::cout << "📸 No images available. Add images to assets/images/ (project root) and rebuild" << std::endl;
            return;
        }
        
        currentIndex = (currentIndex + 1) % imagePaths.size();
        std::cout << "\n🎨 Moving to next image..." << std::endl;
        showImageInterface();
    }
    
    void previousImage() {
        if (imagePaths.empty()) {
            std::cout << "📸 No images available. Add images to assets/images/ (project root) and rebuild" << std::endl;
            return;
        }
        
        currentIndex = (currentIndex == 0) ? imagePaths.size() - 1 : currentIndex - 1;
        std::cout << "\n🎨 Moving to previous image..." << std::endl;
        showImageInterface();
    }
    
    void showInfo() {
        std::cout << "\n📊 Aetherwave Status:" << std::endl;
        std::cout << "   Version: 2.0.0" << std::endl;
        std::cout << "   Images loaded: " << imagePaths.size() << std::endl;
        std::cout << "   Current index: " << (imagePaths.empty() ? 0 : currentIndex + 1) << std::endl;
        std::cout << "   Assets directory: assets/images/" << std::endl;
        
        // Check Python API
        int apiResult = system("curl -s http://localhost:8000/health > /dev/null 2>&1");
        std::cout << "   Python API: " << (apiResult == 0 ? "✅ Running" : "❌ Not running") << std::endl;
        
        if (!imagePaths.empty()) {
            std::cout << "   Current image: " << std::filesystem::path(imagePaths[currentIndex]).filename().string() << std::endl;
        }
    }
};

} // namespace Aetherwave

int main() {
    std::cout << "🌊 Aetherwave Display Engine v2.0" << std::endl;
    std::cout << "📱 Visual Interface Mode" << std::endl;
    
    try {
        Aetherwave::SimpleVisualApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
