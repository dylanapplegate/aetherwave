#!/bin/bash

# Aetherwave Phase 4B.2 Test Script
# Tests the theme-driven transition system

set -e

echo "🎨 Testing Aetherwave Phase 4B.2: Advanced Transition System"
echo "==========================================================="

# Check if we're in the project root
if [ ! -f "docker-compose.yml" ] || [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Please run this script from the Aetherwave project root directory"
    exit 1
fi

echo "📋 Test Checklist:"
echo "- Python API running for theme detection"
echo "- C++ application compiles successfully"
echo "- Theme integration working"
echo "- Advanced transitions available"
echo ""

# Start Python API if not running
echo "🐍 Checking Python classification API..."
if ! curl -s http://localhost:8000/health &>/dev/null; then
    echo "   Starting Python API..."
    ./scripts/dev-setup.sh

    # Wait for API to be ready
    echo "   Waiting for API to start..."
    for i in {1..30}; do
        if curl -s http://localhost:8000/health &>/dev/null; then
            echo "   ✅ Python API is running"
            break
        fi
        sleep 1
        if [ $i -eq 30 ]; then
            echo "   ❌ Python API failed to start within 30 seconds"
            exit 1
        fi
    done
else
    echo "   ✅ Python API already running"
fi

# Test theme endpoint
echo ""
echo "🎭 Testing theme detection endpoint..."
THEME_RESPONSE=$(curl -s http://localhost:8000/analyze/collection-theme || echo "ERROR")
if [ "$THEME_RESPONSE" = "ERROR" ]; then
    echo "   ❌ Failed to connect to theme endpoint"
    exit 1
else
    echo "   ✅ Theme endpoint responding"
    echo "   Response preview: ${THEME_RESPONSE:0:100}..."
fi

# Build C++ application
echo ""
echo "🔨 Building C++ application with new theme system..."
./scripts/cpp-setup.sh

# Check if build was successful
if [ ! -f "build/bin/Aetherwave" ]; then
    echo "❌ C++ build failed"
    exit 1
fi

echo "✅ C++ application built successfully"

# Create sample images if none exist
echo ""
echo "📸 Checking for test images..."
mkdir -p build/bin/assets/images

if [ -z "$(ls -A build/bin/assets/images 2>/dev/null)" ]; then
    echo "   Creating sample images for testing..."

    # Copy any images from project assets if they exist
    if [ -d "assets" ] && [ "$(ls -A assets 2>/dev/null)" ]; then
        cp assets/* build/bin/assets/images/ 2>/dev/null || true
        echo "   ✅ Copied project assets"
    else
        # Create placeholder text files to represent images for testing
        cat > build/bin/assets/images/README_TESTING.txt << EOF
For full testing, please add actual image files to this directory.

Supported formats: .jpg, .jpeg, .png, .gif, .bmp, .tiff, .tif

The application will load and display these images with theme-appropriate transitions:
- Cyberfemme content → Glitch transitions with chromatic aberration
- Organic content → Soft fades with warm blends
- Tech content → Sharp pixel transitions
- Other content → Smart fallback transitions

Add your images here and restart the application to see the theme system in action!
EOF
        echo "   ⚠️  No images found. Added instructions for adding test images."
    fi
else
    IMAGE_COUNT=$(ls -1 build/bin/assets/images/* 2>/dev/null | wc -l | tr -d ' ')
    echo "   ✅ Found $IMAGE_COUNT files in assets directory"
fi

echo ""
echo "🎉 Phase 4B.2 Test Setup Complete!"
echo ""
echo "📋 What was implemented:"
echo "   ✅ ThemeManager with Python API integration"
echo "   ✅ Theme-aware transition system"
echo "   ✅ Advanced transition effects:"
echo "       - Glitch (cyberfemme): chromatic aberration, digital artifacts"
echo "       - Soft fade (organic): eased timing, warm blends"
echo "       - Pixel (tech): sharp digital transitions"
echo "       - Dissolve (tech): pixelated digital effects"
echo "   ✅ Automatic transition selection based on content analysis"
echo "   ✅ Fallback system for offline operation"
echo ""
echo "🚀 To test the implementation:"
echo "   1. Run: ./scripts/cpp-run.sh"
echo "   2. Press 'T' to toggle theme debug overlay"
echo "   3. Press 'Space' to cycle images and see theme-driven transitions"
echo "   4. Watch the transitions adapt to your content!"
echo ""
echo "🎮 Controls in the application:"
echo "   - T: Theme debug overlay"
echo "   - P: Performance overlay"
echo "   - Space: Next image"
echo "   - F: Toggle fullscreen"
echo "   - ESC: Exit"
echo ""
echo "🧠 The theme system will:"
echo "   - Analyze your image collection via Python API"
echo "   - Detect dominant themes (cyberfemme, organic, tech, etc.)"
echo "   - Automatically select appropriate transitions"
echo "   - Update in real-time as you add new images"
