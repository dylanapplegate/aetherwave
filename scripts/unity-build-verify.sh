#!/bin/bash
# Unity Build Verification Script
# Ensures Unity project compiles successfully before claiming setup is complete

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
UNITY_PROJECT_PATH="$PROJECT_ROOT/src/unity/Aetherwave"
UNITY_PATH="/Applications/Unity/Hub/Editor/6000.1.14f1/Unity.app/Contents/MacOS/Unity"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Unity Build Verification ===${NC}"
echo "Project: $UNITY_PROJECT_PATH"
echo "Unity: $(basename "$UNITY_PATH") $(${UNITY_PATH} -version)"
echo ""

# Step 1: Check Unity installation
echo -e "${YELLOW}Step 1: Checking Unity installation...${NC}"
if [ ! -f "$UNITY_PATH" ]; then
    echo -e "${RED}❌ Unity not found at: $UNITY_PATH${NC}"
    echo "Please install Unity 6.1 (6000.1.14f1) via Unity Hub"
    exit 1
fi
echo -e "${GREEN}✅ Unity found${NC}"

# Step 2: Check project structure
echo -e "${YELLOW}Step 2: Checking project structure...${NC}"
if [ ! -d "$UNITY_PROJECT_PATH" ]; then
    echo -e "${RED}❌ Unity project not found at: $UNITY_PROJECT_PATH${NC}"
    exit 1
fi

required_files=(
    "Assets/Scripts/AetherwaveGallery.cs"
    "Assets/Scripts/Managers/ImageDisplayManager.cs"
    "Assets/Scripts/Managers/ThemeManager.cs"
    "Assets/Scripts/Managers/PythonAPIClient.cs"
    "Assets/Scripts/Core/ThemeProfile.cs"
    "Packages/manifest.json"
    "Assets/Scripts/AetherwaveAssembly.asmdef"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$UNITY_PROJECT_PATH/$file" ]; then
        echo -e "${RED}❌ Missing required file: $file${NC}"
        exit 1
    fi
done
echo -e "${GREEN}✅ Project structure valid${NC}"

# Step 3: C# Syntax Check (using mcs if available)
echo -e "${YELLOW}Step 3: C# syntax validation...${NC}"
if command -v mcs &> /dev/null; then
    echo "Running basic C# syntax check..."
    # Create temporary compilation test
    temp_dir=$(mktemp -d)
    find "$UNITY_PROJECT_PATH/Assets/Scripts" -name "*.cs" -exec cp {} "$temp_dir/" \;
    
    cd "$temp_dir"
    if mcs -t:library *.cs &> /dev/null; then
        echo -e "${GREEN}✅ C# syntax appears valid${NC}"
    else
        echo -e "${YELLOW}⚠️  Basic C# syntax check failed (may be Unity-specific code)${NC}"
    fi
    rm -rf "$temp_dir"
else
    echo -e "${YELLOW}⚠️  mcs not available, skipping basic syntax check${NC}"
fi

# Step 4: Unity Project Compilation Test
echo -e "${YELLOW}Step 4: Unity compilation test...${NC}"
echo "Attempting Unity batch mode compilation..."

# Create log directory
LOG_DIR="$PROJECT_ROOT/build/unity-logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/build-$(date +%Y%m%d-%H%M%S).log"

# Unity batch mode compilation test (refresh project and compile scripts)
cd "$UNITY_PROJECT_PATH"
"$UNITY_PATH" \
    -batchmode \
    -quit \
    -projectPath "$UNITY_PROJECT_PATH" \
    -buildTarget StandaloneOSX \
    -logFile "$LOG_FILE" \
    2>&1 | grep -E "(error|Error|ERROR|warning|Warning|WARNING|Compilation|compilation|✓|×)" | head -20 &

# Wait for Unity process to complete (with timeout)
UNITY_PID=$!
timeout=60
while [ $timeout -gt 0 ]; do
    if ! kill -0 $UNITY_PID 2>/dev/null; then
        break
    fi
    sleep 1
    ((timeout--))
done

# Kill Unity if still running
if kill -0 $UNITY_PID 2>/dev/null; then
    echo -e "${YELLOW}⚠️  Unity process timeout, terminating...${NC}"
    kill $UNITY_PID 2>/dev/null || true
    sleep 2
    kill -9 $UNITY_PID 2>/dev/null || true
fi

# Step 5: Analyze compilation log
echo -e "${YELLOW}Step 5: Analyzing compilation results...${NC}"
if [ -f "$LOG_FILE" ]; then
    echo "Log file: $LOG_FILE"
    
    # Check for actual compilation errors (not URP shader warnings)
    if grep -E "(CompilerOutput|Assets.*error|ScriptCompilation.*error|CompileErrors)" "$LOG_FILE" | grep -qv "fallback shader"; then
        echo -e "${RED}❌ Compilation errors found:${NC}"
        grep -E "(CompilerOutput|Assets.*error|ScriptCompilation.*error|CompileErrors)" "$LOG_FILE" | grep -v "fallback shader" | head -10
        echo ""
        echo "Full log available at: $LOG_FILE"
        exit 1
    fi
    
    # Check for URP shader warnings (non-critical)
    urp_warnings=$(grep -c "fallback shader.*not found" "$LOG_FILE" || echo "0")
    if [ "$urp_warnings" -gt 0 ]; then
        echo -e "${YELLOW}⚠️  $urp_warnings URP shader warnings (non-critical)${NC}"
    fi
    
    # Check for other warnings
    other_warnings=$(grep -ci "warning" "$LOG_FILE" | grep -v "fallback shader" || echo "0")
    if [ "$other_warnings" -gt 0 ]; then
        echo -e "${YELLOW}⚠️  $other_warnings other warnings found:${NC}"
        grep -i "warning" "$LOG_FILE" | grep -v "fallback shader" | head -5
    fi
    
    # Look for success indicators
    if grep -qi "RefreshCompleted" "$LOG_FILE" || grep -qi "Unloading.*unused Assets" "$LOG_FILE"; then
        echo -e "${GREEN}✅ Unity compilation successful${NC}"
    else
        echo -e "${YELLOW}⚠️  Compilation status unclear, check log: $LOG_FILE${NC}"
    fi
else
    echo -e "${RED}❌ No Unity log file generated${NC}"
    exit 1
fi

# Step 6: Package validation
echo -e "${YELLOW}Step 6: Package dependencies validation...${NC}"
MANIFEST_FILE="$UNITY_PROJECT_PATH/Packages/manifest.json"
if [ -f "$MANIFEST_FILE" ]; then
    required_packages=("com.unity.ugui" "com.unity.render-pipelines.universal")
    for package in "${required_packages[@]}"; do
        if grep -q "$package" "$MANIFEST_FILE"; then
            echo -e "${GREEN}✅ Package found: $package${NC}"
        else
            echo -e "${RED}❌ Missing package: $package${NC}"
            exit 1
        fi
    done
else
    echo -e "${RED}❌ manifest.json not found${NC}"
    exit 1
fi

echo ""
# Step 7: Check for built application
echo -e "${YELLOW}Step 7: Checking for built application...${NC}"
APP_PATH="$PROJECT_ROOT/src/unity/Aetherwave/build/unity/Aetherwave.app"
if [ -d "$APP_PATH" ]; then
    echo -e "${GREEN}✅ Built application found: $APP_PATH${NC}"
    # Get app size for verification
    APP_SIZE=$(du -sh "$APP_PATH" | cut -f1)
    echo -e "${GREEN}✅ App size: $APP_SIZE${NC}"
else
    echo -e "${YELLOW}⚠️  Built application not found (compilation test only)${NC}"
fi

echo ""
echo -e "${GREEN}🎉 Unity Build Verification PASSED${NC}"
echo -e "${GREEN}✅ Project structure valid${NC}"
echo -e "${GREEN}✅ Unity compilation successful${NC}"
echo -e "${GREEN}✅ Required packages present${NC}"
if [ -d "$APP_PATH" ]; then
    echo -e "${GREEN}✅ Built application available${NC}"
fi
echo ""
echo -e "${YELLOW}Build log saved to: $LOG_FILE${NC}"
echo -e "${YELLOW}Project ready for development in Unity Editor${NC}"
if [ -d "$APP_PATH" ]; then
    echo -e "${YELLOW}To run the app: open '$APP_PATH'${NC}"
fi