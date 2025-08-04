#!/bin/bash

# ============================================================================
# Aetherwave Unity Installation Verification Script
# ============================================================================
# Verifies Unity Hub and Unity 2022.3 LTS installation

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 Aetherwave Unity Installation Verification${NC}"
echo "=================================================="

# Check Homebrew
echo -e "${BLUE}📦 Checking Homebrew...${NC}"
if command -v brew >/dev/null 2>&1; then
    echo -e "${GREEN}✅ Homebrew installed${NC}"
    BREW_VERSION=$(brew --version | head -1)
    echo "   $BREW_VERSION"
else
    echo -e "${YELLOW}⚠️  Homebrew not found${NC}"
    echo "   Install with: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
fi

echo

# Check Unity Hub
echo -e "${BLUE}🎮 Checking Unity Hub...${NC}"
UNITY_HUB_PATH="/Applications/Unity Hub.app/Contents/MacOS/Unity Hub"
if [[ -f "$UNITY_HUB_PATH" ]]; then
    echo -e "${GREEN}✅ Unity Hub installed${NC}"
    echo "   Location: /Applications/Unity Hub.app"
    
    # Try to get version
    if command -v "/Applications/Unity Hub.app/Contents/MacOS/Unity Hub" >/dev/null 2>&1; then
        echo "   Status: Ready"
    fi
else
    echo -e "${RED}❌ Unity Hub not found${NC}"
    echo "   Install with: brew install --cask unity-hub"
fi

echo

# Check Unity 2022.3 LTS
echo -e "${BLUE}🔧 Checking Unity 2022.3 LTS...${NC}"
UNITY_2022_PATH=$(find /Applications/Unity/Hub/Editor -name "2022.3.*" -type d 2>/dev/null | head -1)
if [[ -n "$UNITY_2022_PATH" ]]; then
    echo -e "${GREEN}✅ Unity 2022.3 LTS installed${NC}"
    echo "   Location: $UNITY_2022_PATH"
    
    # Check Unity version
    UNITY_BINARY="$UNITY_2022_PATH/Unity.app/Contents/MacOS/Unity"
    if [[ -f "$UNITY_BINARY" ]]; then
        echo "   Binary: Ready"
    fi
else
    echo -e "${RED}❌ Unity 2022.3 LTS not found${NC}"
    echo "   Install through Unity Hub:"
    echo "   1. Open Unity Hub: open -a 'Unity Hub'"
    echo "   2. Installs tab -> Install Editor -> Unity 2022.3 LTS"
fi

echo

# Check Xcode Command Line Tools
echo -e "${BLUE}🛠️  Checking Xcode Command Line Tools...${NC}"
if xcode-select -p >/dev/null 2>&1; then
    echo -e "${GREEN}✅ Xcode Command Line Tools installed${NC}"
    XCODE_PATH=$(xcode-select -p)
    echo "   Location: $XCODE_PATH"
else
    echo -e "${RED}❌ Xcode Command Line Tools not found${NC}"
    echo "   Install with: xcode-select --install"
fi

echo

# Check system requirements
echo -e "${BLUE}💻 Checking System Requirements...${NC}"

# macOS version
MACOS_VERSION=$(sw_vers -productVersion)
echo "   macOS: $MACOS_VERSION"

# RAM
RAM_GB=$(sysctl -n hw.memsize | awk '{print int($1/1024/1024/1024)}')
echo "   RAM: ${RAM_GB}GB"
if [[ $RAM_GB -ge 8 ]]; then
    echo -e "   ${GREEN}✅ RAM sufficient for large collections${NC}"
elif [[ $RAM_GB -ge 4 ]]; then
    echo -e "   ${YELLOW}⚠️  RAM adequate (4GB+), 8GB+ recommended${NC}"
else
    echo -e "   ${RED}❌ RAM insufficient (less than 4GB)${NC}"
fi

# Disk space check
AVAILABLE_GB=$(df -g . | tail -1 | awk '{print $4}')
echo "   Available Disk Space: ${AVAILABLE_GB}GB"
if [[ $AVAILABLE_GB -ge 10 ]]; then
    echo -e "   ${GREEN}✅ Disk space sufficient${NC}"
else
    echo -e "   ${YELLOW}⚠️  Low disk space (less than 10GB available)${NC}"
fi

echo
echo "=================================================="

# Overall status
if [[ -f "$UNITY_HUB_PATH" ]] && [[ -n "$UNITY_2022_PATH" ]] && xcode-select -p >/dev/null 2>&1; then
    echo -e "${GREEN}🎉 All requirements met! Ready to run Unity setup.${NC}"
    echo
    echo "Next steps:"
    echo "1. Run Unity setup: ./scripts/unity-setup.sh"
    echo "2. Launch Unity project: ./scripts/unity-run.sh"
else
    echo -e "${YELLOW}⚠️  Some requirements missing. Follow the installation steps above.${NC}"
    echo
    echo "Quick setup commands:"
    if [[ ! -f "$UNITY_HUB_PATH" ]]; then
        echo "   brew install --cask unity-hub"
    fi
    if ! xcode-select -p >/dev/null 2>&1; then
        echo "   xcode-select --install"
    fi
    if [[ -z "$UNITY_2022_PATH" ]]; then
        echo "   open -a 'Unity Hub'  # Then install Unity 2022.3 LTS"
    fi
fi