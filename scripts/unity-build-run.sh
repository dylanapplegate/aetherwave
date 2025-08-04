#!/bin/bash

# ============================================================================
# Aetherwave Unity Build & Run Script
# ============================================================================
# Builds Unity project and runs the gallery app

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}🔨 Building and Running Aetherwave Unity Gallery${NC}"

# Check if Unity project exists
if [[ ! -d "src/unity/Aetherwave" ]]; then
    echo -e "${RED}❌ Unity project not found. Run ./scripts/unity-setup.sh first${NC}"
    exit 1
fi

# Find Unity installation
UNITY_6_PATH=$(find /Applications/Unity/Hub/Editor -name "6000.1.*" -o -name "6.0.*" -type d 2>/dev/null | head -1)
if [[ -z "$UNITY_6_PATH" ]]; then
    echo -e "${RED}❌ Unity 6.1 not found. Please install through Unity Hub${NC}"
    exit 1
fi

UNITY_PATH="$UNITY_6_PATH/Unity.app/Contents/MacOS/Unity"
PROJECT_PATH="$(pwd)/src/unity/Aetherwave"
BUILD_PATH="$(pwd)/build/unity"

echo -e "${GREEN}🚀 Building Unity project...${NC}"
echo "Project: $PROJECT_PATH"
echo "Build: $BUILD_PATH"

# Create build directory
mkdir -p "$BUILD_PATH"

# Build the project
echo -e "${BLUE}📦 Starting Unity build...${NC}"
"$UNITY_PATH" \
    -projectPath "$PROJECT_PATH" \
    -buildTarget StandaloneOSX \
    -buildPath "$BUILD_PATH/Aetherwave.app" \
    -executeMethod UnityEditor.BuildPipeline.BuildPlayer \
    -quit \
    -batchmode \
    -nographics \
    -logFile build/unity-build.log

if [[ $? -eq 0 ]]; then
    echo -e "${GREEN}✅ Unity build completed successfully${NC}"
    
    # Check if app was created
    if [[ -d "$BUILD_PATH/Aetherwave.app" ]]; then
        echo -e "${GREEN}🎮 Launching Aetherwave Gallery...${NC}"
        open "$BUILD_PATH/Aetherwave.app"
        echo -e "${GREEN}✅ Gallery app launched!${NC}"
        echo
        echo "Controls:"
        echo "  SPACE - Next image"
        echo "  BACKSPACE - Previous image"
        echo "  T - Toggle theme debug"
        echo "  ESC - Exit"
    else
        echo -e "${RED}❌ Build failed - app not found${NC}"
        echo "Check build log: build/unity-build.log"
        exit 1
    fi
else
    echo -e "${RED}❌ Unity build failed${NC}"
    echo "Check build log: build/unity-build.log"
    exit 1
fi
