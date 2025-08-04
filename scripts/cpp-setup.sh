#!/bin/bash

# Aetherwave C++ Setup and Build Script
# One-command setup for the C++ display engine

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🎨 Aetherwave C++ Display Engine Setup"
echo "======================================"

# Check system requirements
echo "🔍 Checking system requirements..."

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "❌ This script is designed for macOS only"
    exit 1
fi

# Check for Xcode command line tools
if ! command -v gcc &> /dev/null; then
    echo "❌ Xcode command line tools not found"
    echo "   Please run: xcode-select --install"
    exit 1
fi

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found"
    echo "   Please install CMake:"
    echo "   - Using Homebrew: brew install cmake"
    echo "   - Download from: https://cmake.org/download/"
    exit 1
fi

echo "✅ System requirements satisfied"

# Navigate to project root
cd "$PROJECT_ROOT"

# Check if openFrameworks is installed
OF_DIR="libs/openFrameworks"
if [ ! -d "$OF_DIR" ]; then
    echo ""
    echo "🔧 openFrameworks not found. Setting up openFrameworks..."
    ./scripts/setup_openframeworks.sh
else
    echo "✅ openFrameworks found"
fi

# Create and configure build directory
echo ""
echo "⚙️  Configuring build environment..."

# Clean previous build if requested
if [ "$1" = "clean" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "📝 Running CMake configuration..."
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DOF_ROOT="../libs/openFrameworks" \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the application
echo ""
echo "🔨 Building Aetherwave C++ application..."
echo "   This may take a few minutes on first build..."

# Use all available CPU cores for faster compilation
NCPU=$(sysctl -n hw.ncpu)
echo "   Using $NCPU CPU cores for parallel compilation"

make -j$NCPU

# Check if build was successful
if [ ! -f "bin/Aetherwave" ]; then
    echo "❌ Build failed - binary not found"
    echo "   Check the error messages above for details"
    exit 1
fi

# Setup runtime environment
echo ""
echo "📂 Setting up runtime environment..."

# Create assets directory structure
mkdir -p bin/assets/images
mkdir -p bin/config

# Copy configuration files
if [ -f "../config/config.yaml" ]; then
    cp ../config/config.yaml bin/config/
    echo "   ✅ Copied config.yaml"
fi

if [ -f "../config/themes.yaml" ]; then
    cp ../config/themes.yaml bin/config/
    echo "   ✅ Copied themes.yaml"
fi

# Note: Asset synchronization is now handled by CMake during build
# Check if source assets exist
if [ -d "../assets/images" ]; then
    SOURCE_IMAGE_COUNT=$(ls -1 ../assets/images/*.{jpg,jpeg,png,gif,bmp,tiff,tif} 2>/dev/null | wc -l | tr -d ' ')
    if [ "$SOURCE_IMAGE_COUNT" -gt 0 ]; then
        echo "   📸 Found $SOURCE_IMAGE_COUNT source images in assets/images/"
        echo "   🔄 Assets synced by CMake build system"
    fi
fi

# Create sample assets directory message (only if no source assets)
if [ ! -d "../assets/images" ] || [ ! "$(ls -A ../assets/images 2>/dev/null)" ]; then
    cat > bin/assets/images/README.txt << EOF
Place your images in this directory for display in Aetherwave.

Supported formats:
- JPEG (.jpg, .jpeg)
- PNG (.png)
- GIF (.gif)
- TIFF (.tiff, .tif)
- BMP (.bmp)

The application will automatically detect and load all images from this directory.
EOF
fi

echo ""
echo "🎉 Build completed successfully!"
echo ""
echo "📍 Binary location: build/bin/Aetherwave"
echo "📁 Runtime assets: build/bin/assets/images/"
echo "📂 Source assets: assets/images/ (add images here)"
echo "⚙️  Config directory: build/bin/config/"
echo ""
echo "🚀 To run the application:"
echo "   Option 1: ./scripts/cpp-run.sh"
echo "   Option 2: cd build/bin && ./Aetherwave"
echo ""
echo "🎮 Controls when running:"
echo "   - SPACE or →: Next image"
echo "   - ← or ↑: Previous image"
echo "   - F: Toggle fullscreen"
echo "   - P: Performance overlay"
echo "   - ESC: Exit"
echo ""
echo "💡 Tips:"
echo "   - Add images to assets/images/ in project root, then rebuild"
echo "   - Images are automatically synced during build process"
echo "   - The app starts in fullscreen mode for best experience"
echo "   - Use 'clean' argument to rebuild from scratch: ./scripts/cpp-setup.sh clean"
