#!/bin/bash
set -e

echo "🌊 Setting up Aetherwave development environment..."

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is required but not installed. Please install Docker Desktop."
    exit 1
fi

# Check if Docker Compose is available
if ! command -v docker-compose &> /dev/null; then
    echo "❌ Docker Compose is required but not installed."
    exit 1
fi

# Create necessary directories
mkdir -p assets config tests

# Create default config if it doesn't exist
if [ ! -f "config/themes.yaml" ]; then
    echo "📝 Creating default theme configuration..."
    cat > config/themes.yaml << EOF
themes:
  cyberfemme:
    colors: ["#ff0080", "#00ffff", "#8000ff"]
    mood: "energetic"
    transitions: ["fade", "slide"]
  minimal:
    colors: ["#ffffff", "#000000", "#808080"]
    mood: "calm"
    transitions: ["fade"]
EOF
fi

# Build and start services
echo "🐳 Building Docker containers..."
docker-compose build

echo "🚀 Starting development environment..."
docker-compose up -d

echo "✅ Aetherwave is ready!"
echo "📡 Classification API: http://localhost:8000"
echo "📚 API Documentation: http://localhost:8000/docs"
echo ""
echo "Next steps:"
echo "  - Place your images in the 'assets' folder"
echo "  - Run 'docker-compose logs -f' to see live logs"
echo "  - Run 'scripts/dev-stop.sh' to stop services"
