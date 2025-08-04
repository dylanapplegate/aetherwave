#!/bin/bash

# Aetherwave Display Engine Startup Script
# Builds and runs the C++ display application

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🎨 Starting Aetherwave Display Engine"
echo "====================================="

# Navigate to project root
cd "$PROJECT_ROOT"

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "📁 Creating build directory..."
    mkdir build
fi

cd build

# Configure CMake if needed
if [ ! -f "Makefile" ]; then
    echo "⚙️  Configuring CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
fi

# Build the application
echo "🔨 Building Aetherwave..."
make -j$(sysctl -n hw.ncpu)

# Check if binary exists
if [ ! -f "bin/Aetherwave" ]; then
    echo "❌ Build failed - binary not found"
    exit 1
fi

# Create assets directory if it doesn't exist
mkdir -p bin/assets/images

# Copy config files
if [ -f "../config/config.yaml" ]; then
    cp ../config/config.yaml bin/
fi

echo "✅ Build successful!"
echo "🚀 Starting display engine..."
echo "   Add images to build/bin/assets/images/ for display"
echo "   Press 'f' to toggle fullscreen, 'ESC' to exit"
echo ""

# Navigate to binary directory and run
cd bin
./Aetherwave
