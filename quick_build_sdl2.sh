#!/bin/bash

# Quick SDL2 setup and build script
echo "🌊 Building Aetherwave with SDL2 graphics engine..."

# Install SDL2 if needed
if ! pkg-config --exists sdl2; then
    echo "📦 Installing SDL2..."
    brew install sdl2 sdl2_image
fi

# Navigate to project
cd /Users/dylanapplegate/Development/aetherwave

# Clean and build
rm -rf build
mkdir -p build
cd build

echo "⚙️ Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "🔨 Building..."
make -j$(sysctl -n hw.ncpu)

if [ -f "bin/Aetherwave" ]; then
    echo "✅ Build successful!"
    echo "🚀 Run: cd build/bin && ./Aetherwave"
else
    echo "❌ Build failed"
    exit 1
fi
