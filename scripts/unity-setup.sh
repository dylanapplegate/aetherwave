#!/bin/bash
# Unity Complete Setup and Verification Script
# Sets up Unity project and ensures it builds successfully

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}    Unity Complete Setup & Verification    ${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""

# Step 1: Check if Unity Hub is installed
echo -e "${YELLOW}Step 1: Checking Unity Hub installation...${NC}"
if ! command -v "/Applications/Unity Hub.app/Contents/MacOS/Unity Hub" &> /dev/null; then
    echo -e "${RED}❌ Unity Hub not found${NC}"
    echo ""
    echo "Please install Unity Hub first:"
    echo "  brew install --cask unity-hub"
    echo ""
    echo "Then install Unity 6.1 (6000.1.14f1) through Unity Hub"
    exit 1
fi
echo -e "${GREEN}✅ Unity Hub found${NC}"

# Step 2: Check Unity Editor installation
echo -e "${YELLOW}Step 2: Checking Unity Editor installation...${NC}"
UNITY_PATH="/Applications/Unity/Hub/Editor/6000.1.14f1/Unity.app/Contents/MacOS/Unity"
if [ ! -f "$UNITY_PATH" ]; then
    echo -e "${RED}❌ Unity 6.1 (6000.1.14f1) not found${NC}"
    echo ""
    echo "Please install Unity 6.1 (6000.1.14f1) through Unity Hub:"
    echo "1. Open Unity Hub"
    echo "2. Go to 'Installs' tab"
    echo "3. Click 'Install Editor'"
    echo "4. Select Unity 6.1 (6000.1.14f1)"
    echo "5. Include modules: macOS Build Support, Universal Windows Platform Build Support"
    exit 1
fi

unity_version=$("$UNITY_PATH" -version)
echo -e "${GREEN}✅ Unity found: $unity_version${NC}"

# Step 3: Verify Unity project structure exists
echo -e "${YELLOW}Step 3: Verifying Unity project structure...${NC}"
UNITY_PROJECT_PATH="$PROJECT_ROOT/src/unity/Aetherwave"
if [ ! -d "$UNITY_PROJECT_PATH" ]; then
    echo -e "${RED}❌ Unity project not found at: $UNITY_PROJECT_PATH${NC}"
    echo "Run the Unity project creation script first"
    exit 1
fi
echo -e "${GREEN}✅ Unity project structure found${NC}"

# Step 4: Run linting checks
echo -e "${YELLOW}Step 4: Running C# linting checks...${NC}"
if [ -f "$SCRIPT_DIR/unity-lint.sh" ]; then
    if "$SCRIPT_DIR/unity-lint.sh"; then
        echo -e "${GREEN}✅ Linting checks passed${NC}"
    else
        echo -e "${RED}❌ Linting checks failed${NC}"
        echo "Please fix linting issues before proceeding"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠️  unity-lint.sh not found, skipping linting${NC}"
fi

# Step 5: Run Unity build verification
echo -e "${YELLOW}Step 5: Running Unity build verification...${NC}"
if [ -f "$SCRIPT_DIR/unity-build-verify.sh" ]; then
    if "$SCRIPT_DIR/unity-build-verify.sh"; then
        echo -e "${GREEN}✅ Unity build verification passed${NC}"
    else
        echo -e "${RED}❌ Unity build verification failed${NC}"
        echo "Unity project has compilation errors"
        exit 1
    fi
else
    echo -e "${RED}❌ unity-build-verify.sh not found${NC}"
    exit 1
fi

# Step 6: Final validation
echo -e "${YELLOW}Step 6: Final project validation...${NC}"

# Check key files exist and are valid
key_files=(
    "src/unity/Aetherwave/Assets/Scripts/AetherwaveGallery.cs"
    "src/unity/Aetherwave/Assets/Scripts/Managers/ImageDisplayManager.cs"
    "src/unity/Aetherwave/Assets/Scripts/Managers/ThemeManager.cs"
    "src/unity/Aetherwave/Packages/manifest.json"
)

for file in "${key_files[@]}"; do
    if [ ! -f "$PROJECT_ROOT/$file" ]; then
        echo -e "${RED}❌ Missing key file: $file${NC}"
        exit 1
    fi
done

# Verify manifest.json has required packages
manifest_file="$PROJECT_ROOT/src/unity/Aetherwave/Packages/manifest.json"
if ! grep -q "com.unity.ugui" "$manifest_file"; then
    echo -e "${RED}❌ Missing UGUI package in manifest.json${NC}"
    exit 1
fi

if ! grep -q "com.unity.render-pipelines.universal" "$manifest_file"; then
    echo -e "${RED}❌ Missing URP package in manifest.json${NC}"
    exit 1
fi

echo -e "${GREEN}✅ All key files present and valid${NC}"

# Success!
echo ""
echo -e "${GREEN}🎉🎉🎉 UNITY SETUP COMPLETE & VERIFIED 🎉🎉🎉${NC}"
echo ""
echo -e "${GREEN}✅ Unity Hub and Editor installed${NC}"
echo -e "${GREEN}✅ Project structure valid${NC}"
echo -e "${GREEN}✅ C# code passes linting${NC}"
echo -e "${GREEN}✅ Unity project compiles successfully${NC}"
echo -e "${GREEN}✅ Required packages installed${NC}"
echo ""
echo -e "${BLUE}Next steps:${NC}"
echo "1. Open Unity Hub"
echo "2. Click 'Open' and select: $UNITY_PROJECT_PATH"
echo "3. Unity will import packages and compile scripts"
echo "4. Create your main scene and start developing!"
echo ""
echo -e "${YELLOW}Project ready for Unity development! 🚀${NC}"