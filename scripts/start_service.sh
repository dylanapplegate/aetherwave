#!/bin/bash

# Aetherwave Service Startup Script
# Starts the Python classification service with proper environment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🌊 Starting Aetherwave Classification Service"
echo "============================================"

# Navigate to project root
cd "$PROJECT_ROOT"

# Check if virtual environment exists
if [ ! -d ".venv" ]; then
    echo "❌ Virtual environment not found"
    echo "   Please run: python3 -m venv .venv && source .venv/bin/activate && pip install -r src/python/requirements.txt"
    exit 1
fi

# Activate virtual environment
echo "📦 Activating virtual environment..."
source .venv/bin/activate

# Check if dependencies are installed
if ! python -c "import fastapi, uvicorn, cv2, colorthief" 2>/dev/null; then
    echo "📥 Installing dependencies..."
    pip install -r src/python/requirements.txt
fi

# Create necessary directories
mkdir -p config/metadata
mkdir -p assets/images

# Start the service
echo "🚀 Starting classification service on http://127.0.0.1:8000"
echo "   Access documentation at: http://127.0.0.1:8000/docs"
echo "   Press Ctrl+C to stop"
echo ""

cd src/python
python main.py
