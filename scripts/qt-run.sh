#!/bin/bash

# Aetherwave Qt Frontend Startup Script
# Starts the Qt gallery application with proper error handling

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🎨 Starting Aetherwave Qt Frontend${NC}"

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Check if we're in the right directory
if [[ ! -f "$PROJECT_ROOT/src/fe/main.py" ]]; then
    echo -e "${RED}❌ Error: Qt frontend not found at $PROJECT_ROOT/src/fe/main.py${NC}"
    exit 1
fi

# Check Python dependencies
echo -e "${BLUE}🔍 Checking Python dependencies...${NC}"
if ! python -c "import PySide6" 2>/dev/null; then
    echo -e "${YELLOW}⚠️  Installing PySide6...${NC}"
    pip install PySide6 requests pillow
fi

# Check if FastAPI backend is running
echo -e "${BLUE}🔍 Checking FastAPI backend...${NC}"
if ! curl -s http://localhost:8000/health > /dev/null; then
    echo -e "${YELLOW}⚠️  FastAPI backend not running. Starting it...${NC}"
    
    # Start the backend in the background
    cd "$PROJECT_ROOT"
    if [[ -f "docker-compose.yml" ]]; then
        echo -e "${BLUE}🐳 Starting Docker services...${NC}"
        ./scripts/dev-setup.sh
    else
        echo -e "${BLUE}🐍 Starting Python backend...${NC}"
        python -m uvicorn src.api.main:app --host 0.0.0.0 --port 8000 --reload &
        BACKEND_PID=$!
        
        # Wait for backend to start
        echo -e "${BLUE}⏳ Waiting for backend to start...${NC}"
        for i in {1..30}; do
            if curl -s http://localhost:8000/health > /dev/null; then
                echo -e "${GREEN}✅ Backend started successfully${NC}"
                break
            fi
            sleep 1
        done
        
        if ! curl -s http://localhost:8000/health > /dev/null; then
            echo -e "${RED}❌ Failed to start backend${NC}"
            exit 1
        fi
    fi
else
    echo -e "${GREEN}✅ FastAPI backend is running${NC}"
fi

# Create logs directory
mkdir -p "$PROJECT_ROOT/build/logs"

# Start the Qt application
echo -e "${BLUE}🎨 Starting Qt Gallery...${NC}"
cd "$PROJECT_ROOT/src/fe"

# Set Qt environment variables for better rendering
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_ENABLE_HIGHDPI_SCALING=1

# Start the application
python main.py

# Cleanup on exit
if [[ -n "$BACKEND_PID" ]]; then
    echo -e "${BLUE}🧹 Stopping backend process...${NC}"
    kill $BACKEND_PID 2>/dev/null || true
fi

echo -e "${GREEN}✅ Aetherwave Qt Frontend stopped${NC}"