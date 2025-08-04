#!/bin/bash

# Aetherwave C++ Application Runner  
# Quick run script for the C++ display engine

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🎨 Starting Aetherwave Display Engine"
echo "====================================="

# Navigate to project root
cd "$PROJECT_ROOT"

# Check if binary exists
if [ ! -f "build/bin/Aetherwave" ]; then
    echo "❌ Aetherwave binary not found"
    echo "   Run './scripts/cpp-setup.sh' first to build the application"
    exit 1
fi

# Check if assets directory has images
ASSETS_DIR="build/bin/assets/images"
SOURCE_ASSETS_DIR="assets/images"

if [ ! -d "$ASSETS_DIR" ] || [ -z "$(ls -A "$ASSETS_DIR" 2>/dev/null | grep -E '\.(jpg|jpeg|png|gif|bmp|tiff|tif)$')" ]; then
    echo "⚠️  No images found in $ASSETS_DIR"
    echo "   The application will run but won't display any images"
    echo ""
    
    if [ -d "$SOURCE_ASSETS_DIR" ] && [ "$(ls -A "$SOURCE_ASSETS_DIR" 2>/dev/null | grep -E '\.(jpg|jpeg|png|gif|bmp|tiff|tif)$')" ]; then
        echo "🔍 Found images in source assets directory ($SOURCE_ASSETS_DIR)"
        echo "   These will be synced during the next build"
        echo "   Run './scripts/cpp-setup.sh' to rebuild and sync assets"
    else
        echo "💡 To add images:"
        echo "   1. Place images in: $SOURCE_ASSETS_DIR"
        echo "   2. Rebuild: ./scripts/cpp-setup.sh"
        echo "   3. Run: ./scripts/cpp-run.sh"
    fi
    echo ""
fi

# Show current configuration
echo "📋 Current Configuration:"
echo "   🎯 Binary: build/bin/Aetherwave"
echo "   📁 Build Assets: $ASSETS_DIR"
echo "   📂 Source Assets: $SOURCE_ASSETS_DIR"

if [ -d "$ASSETS_DIR" ]; then
    IMAGE_COUNT=$(ls -1 "$ASSETS_DIR"/*.{jpg,jpeg,png,gif,bmp,tiff,tif} 2>/dev/null | wc -l | tr -d ' ')
    echo "   📸 Images: $IMAGE_COUNT files"
fi

echo ""
echo "🎮 Controls:"
echo "   - SPACE/→: Next image"
echo "   - ←/↑: Previous image"
echo "   - F: Toggle fullscreen"
echo "   - P: Performance overlay"
echo "   - ESC: Exit"
echo ""

# Check if Python API is running
if curl -s http://localhost:8000/health &>/dev/null; then
    echo "✅ Python classification API is running"
else
    echo "⚠️  Python classification API not detected"
    echo "   Run './scripts/dev-setup.sh' to start the full development environment"
fi

echo ""
echo "🚀 Launching Aetherwave..."
echo "   (Use Ctrl+C to stop this script if the app doesn't start)"
echo ""

# Navigate to the binary directory and run
cd build/bin
exec ./Aetherwave
