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
if [ ! -d "$ASSETS_DIR" ] || [ -z "$(ls -A "$ASSETS_DIR" 2>/dev/null | grep -E '\.(jpg|jpeg|png|gif|bmp|tiff|tif)$')" ]; then
    echo "⚠️  No images found in $ASSETS_DIR"
    echo "   The application will run but won't display any images"
    echo "   Add images to $ASSETS_DIR for the best experience"
    echo ""

    # Offer to copy sample images if they exist
    if [ -d "assets" ] && [ "$(ls -A assets 2>/dev/null | grep -E '\.(jpg|jpeg|png|gif|bmp|tiff|tif)$')" ]; then
        echo "🔍 Found images in project assets directory"
        read -p "   Copy them to the runtime directory? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            cp assets/*.{jpg,jpeg,png,gif,bmp,tiff,tif} "$ASSETS_DIR/" 2>/dev/null || true
            echo "   ✅ Copied images to runtime directory"
        fi
    fi
    echo ""
fi

# Show current configuration
echo "📋 Current Configuration:"
echo "   🎯 Binary: build/bin/Aetherwave"
echo "   📁 Assets: $ASSETS_DIR"

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
