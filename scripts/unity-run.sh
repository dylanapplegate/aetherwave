#!/bin/bash

# ============================================================================
# Aetherwave Unity Runner Script
# ============================================================================
# Launches Unity project for development and testing

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}🎮 Launching Aetherwave Unity Project${NC}"

# Check if Unity project exists
if [[ ! -d "src/unity/Aetherwave" ]]; then
    echo -e "${YELLOW}⚠️  Unity project not found. Run ./scripts/unity-setup.sh first${NC}"
    exit 1
fi

# Find Unity installation
UNITY_6_PATH=$(find /Applications/Unity/Hub/Editor -name "6000.1.*" -o -name "6.0.*" -type d 2>/dev/null | head -1)
if [[ -z "$UNITY_6_PATH" ]]; then
    echo -e "${YELLOW}⚠️  Unity 6.1 not found. Please install through Unity Hub${NC}"
    exit 1
fi

UNITY_PATH="$UNITY_6_PATH/Unity.app/Contents/MacOS/Unity"

echo -e "${GREEN}🚀 Opening Unity project...${NC}"
echo "Project: $(pwd)/src/unity/Aetherwave"

# Launch Unity with the project
"$UNITY_PATH" -projectPath "$(pwd)/src/unity/Aetherwave" &

echo -e "${GREEN}✅ Unity launched successfully${NC}"
echo "The Aetherwave Unity project should open shortly."
echo
echo "Next steps:"
echo "1. Set up Universal Render Pipeline (URP)"
echo "2. Create main scene with gallery UI"
echo "3. Test Python API integration"
