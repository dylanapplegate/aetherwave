#!/bin/bash

# ============================================================================
# Aetherwave Unity App Launcher
# ============================================================================
# Launches the latest built Unity app

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}🚀 Launching Aetherwave Unity Gallery App${NC}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
APP_PATH="$PROJECT_ROOT/src/build/bin/Aetherwave.app"

# Check if app exists
if [ ! -d "$APP_PATH" ]; then
    echo -e "${RED}❌ Unity app not found at: $APP_PATH${NC}"
    echo -e "${YELLOW}Please build the Unity project first with: ./scripts/unity-build-verify.sh${NC}"
    exit 1
fi

# Check app age
APP_AGE=$(stat -f "%m" "$APP_PATH" 2>/dev/null || echo "0")
CURRENT_TIME=$(date +%s)
AGE_HOURS=$(( (CURRENT_TIME - APP_AGE) / 3600 ))

echo -e "${GREEN}✅ Found Unity app: $APP_PATH${NC}"
if [ "$AGE_HOURS" -gt 2 ]; then
    echo -e "${YELLOW}⚠️  App is $AGE_HOURS hours old - consider rebuilding${NC}"
else
    echo -e "${GREEN}✅ App is recent (built $AGE_HOURS hours ago)${NC}"
fi

# Launch the app
echo -e "${BLUE}🎮 Starting Aetherwave Unity Gallery...${NC}"
open "$APP_PATH"

echo -e "${GREEN}✅ Unity app launched successfully${NC}"
echo
echo "Controls:"
echo "  → / SPACE: Next image"
echo "  ← / BACKSPACE: Previous image"
echo "  F: Toggle fullscreen"
echo "  I: Toggle debug info"
echo "  ESC: Exit application"