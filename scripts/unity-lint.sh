#!/bin/bash
# Unity C# Linting Script
# Validates Unity C# code quality and style

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
UNITY_PROJECT_PATH="$PROJECT_ROOT/src/unity/Aetherwave"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Unity C# Linting ===${NC}"
echo "Project: $UNITY_PROJECT_PATH"
echo ""

# Step 1: Find all C# files
echo -e "${YELLOW}Step 1: Scanning C# files...${NC}"
cs_files=$(find "$UNITY_PROJECT_PATH/Assets/Scripts" -name "*.cs" 2>/dev/null || echo "")
if [ -z "$cs_files" ]; then
    echo -e "${RED}❌ No C# files found in Assets/Scripts${NC}"
    exit 1
fi

file_count=$(echo "$cs_files" | wc -l)
echo -e "${GREEN}✅ Found $file_count C# files${NC}"

# Step 2: Basic syntax validation
echo -e "${YELLOW}Step 2: Syntax validation...${NC}"
syntax_errors=0

for file in $cs_files; do
    echo "Checking: $(basename "$file")"
    
    # Check for basic syntax issues
    if ! grep -q "using UnityEngine;" "$file" && ! grep -q "namespace Aetherwave" "$file"; then
        echo -e "${YELLOW}⚠️  Missing Unity namespace or Aetherwave namespace in: $(basename "$file")${NC}"
    fi
    
    # Check for unclosed braces (basic)
    open_braces=$(grep -o "{" "$file" | wc -l)
    close_braces=$(grep -o "}" "$file" | wc -l)
    if [ "$open_braces" -ne "$close_braces" ]; then
        echo -e "${RED}❌ Brace mismatch in: $(basename "$file") (${open_braces} open, ${close_braces} close)${NC}"
        ((syntax_errors++))
    fi
    
    # Check for common Unity patterns
    if grep -q "MonoBehaviour" "$file"; then
        if ! grep -q "Start\|Awake\|Update" "$file"; then
            echo -e "${YELLOW}⚠️  MonoBehaviour without common Unity methods in: $(basename "$file")${NC}"
        fi
    fi
done

if [ $syntax_errors -gt 0 ]; then
    echo -e "${RED}❌ $syntax_errors syntax errors found${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Basic syntax validation passed${NC}"

# Step 3: Coding standards check
echo -e "${YELLOW}Step 3: Coding standards validation...${NC}"
standards_warnings=0

for file in $cs_files; do
    # Check namespace usage
    if ! grep -q "namespace Aetherwave" "$file"; then
        echo -e "${YELLOW}⚠️  Missing Aetherwave namespace in: $(basename "$file")${NC}"
        ((standards_warnings++))
    fi
    
    # Check for TODO/FIXME comments
    if grep -qi "TODO\|FIXME\|HACK" "$file"; then
        echo -e "${YELLOW}⚠️  TODO/FIXME found in: $(basename "$file")${NC}"
        ((standards_warnings++))
    fi
    
    # Check for public fields without [SerializeField]
    if grep -q "public.*;" "$file" && ! grep -q "MonoBehaviour" "$file"; then
        echo -e "${YELLOW}⚠️  Public fields found (consider private with [SerializeField]): $(basename "$file")${NC}"
        ((standards_warnings++))
    fi
done

# Step 4: Unity-specific validation
echo -e "${YELLOW}Step 4: Unity-specific validation...${NC}"
unity_issues=0

# Check assembly definition
asmdef_file="$UNITY_PROJECT_PATH/Assets/Scripts/AetherwaveAssembly.asmdef"
if [ ! -f "$asmdef_file" ]; then
    echo -e "${RED}❌ Missing assembly definition: AetherwaveAssembly.asmdef${NC}"
    ((unity_issues++))
fi

# Check for proper Unity imports
for file in $cs_files; do
    # Files using UI should import UnityEngine.UI
    if grep -q "RawImage\|Text\|Button\|Canvas" "$file" && ! grep -q "using UnityEngine.UI;" "$file"; then
        echo -e "${RED}❌ UI components used without 'using UnityEngine.UI;': $(basename "$file")${NC}"
        ((unity_issues++))
    fi
    
    # Files using URP should import proper namespaces
    if grep -q "UniversalRenderPipelineAsset\|Volume" "$file" && ! grep -q "using UnityEngine.Rendering.Universal;" "$file"; then
        echo -e "${RED}❌ URP components used without proper imports: $(basename "$file")${NC}"
        ((unity_issues++))
    fi
done

# Step 5: Report results
echo ""
echo -e "${YELLOW}=== Linting Results ===${NC}"
if [ $syntax_errors -eq 0 ] && [ $unity_issues -eq 0 ]; then
    echo -e "${GREEN}✅ All critical checks passed${NC}"
    if [ $standards_warnings -gt 0 ]; then
        echo -e "${YELLOW}⚠️  $standards_warnings coding standards warnings (non-critical)${NC}"
    fi
    echo ""
    echo -e "${GREEN}🎉 Unity C# Linting PASSED${NC}"
    exit 0
else
    echo -e "${RED}❌ $syntax_errors syntax errors${NC}"
    echo -e "${RED}❌ $unity_issues Unity-specific issues${NC}"
    echo -e "${YELLOW}⚠️  $standards_warnings coding standards warnings${NC}"
    echo ""
    echo -e "${RED}💥 Unity C# Linting FAILED${NC}"
    exit 1
fi