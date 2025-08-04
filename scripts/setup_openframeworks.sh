#!/bin/bash

# Aetherwave openFrameworks Setup Script
# Automatically downloads and configures openFrameworks for Apple Silicon

set -e

# Configuration
OF_VERSION="0.12.0"
OF_DIR="libs/openFrameworks"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🌊 Aetherwave openFrameworks Setup"
echo "=================================="

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "❌ This script is designed for macOS only"
    exit 1
fi

# Check for Xcode command line tools
if ! command -v gcc &> /dev/null; then
    echo "❌ Xcode command line tools not found"
    echo "   Please run: xcode-select --install"
    exit 1
fi

# Navigate to project root
cd "$PROJECT_ROOT"

# Create libs directory
mkdir -p libs

# Check if openFrameworks already exists
if [ -d "$OF_DIR" ]; then
    echo "⚠️  openFrameworks directory already exists"
    read -p "   Remove and reinstall? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$OF_DIR"
    else
        echo "   Keeping existing installation"
        exit 0
    fi
fi

echo "📥 Downloading openFrameworks $OF_VERSION..."

# Download and extract openFrameworks
cd libs
curl -L "https://github.com/openframeworks/openFrameworks/releases/download/$OF_VERSION/of_v${OF_VERSION}_osx_release.tar.gz" \
    -o "openframeworks.tar.gz"

echo "📦 Extracting openFrameworks..."
tar -xzf "openframeworks.tar.gz"
mv "of_v${OF_VERSION}_osx_release" "openFrameworks"
rm "openframeworks.tar.gz"

cd openFrameworks

echo "🔧 Installing openFrameworks dependencies..."
cd scripts/osx
./install_dependencies.sh

echo "⚙️  Compiling openFrameworks (this may take several minutes)..."
./compileOF.sh

echo "✅ openFrameworks setup complete!"
echo ""
echo "Next steps:"
echo "1. Build the project: mkdir build && cd build && cmake .. && make"
echo "2. Run the application: ./build/bin/Aetherwave"
