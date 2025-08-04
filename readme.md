# **Aetherwave**

Aetherwave is an immersive media engine for displaying cinematic MidJourney art across multiple monitors on macOS. Designed for personal storytelling, it emphasizes **stillness**, **mood**, and **fluidity**, without distraction.

## **🧠 Philosophy**

- Art is the hero—everything else disappears
- Motion and transitions should be **cinematic**, **intentional**, and **low-noise**
- Traditional, handcrafted programming preferred over AI guesswork

## **✨ Features**

- Multi-monitor image/video playback (fullscreen, split-screen, mosaic)
- Classification-based transitions (mood, tone, dominant color)
- Elegant transitions: fade, zoom, slide, drift
- Optional audio sync with bloom/beat detection
- Works with flat folder structures—no manual sorting required

## **🧱 Technologies**

- **C++ / openFrameworks**: Real-time rendering
- **Python**: Metadata extraction, classification
- **Docker**: Containerized development environment
- **YAML / JSON**: Configuration and metadata

## **🚀 Quick Start (Recommended)**

### **Option 1: Docker Setup (Easiest)**

```bash
# Clone and setup
git clone <repo-url> aetherwave
cd aetherwave

# One-command setup
chmod +x scripts/dev-setup.sh
./scripts/dev-setup.sh

# Your classification API is now running at http://localhost:8000
```

**That's it!** The Python classification engine is running. Next:

- Add your images to the `assets/` folder
- Visit http://localhost:8000/docs for API documentation
- Continue with C++ setup below for full rendering

### **Option 2: C++ Display Engine Setup**

For the complete visual experience, you'll also want the C++ display engine:

```bash
# One-command C++ setup (handles openFrameworks installation)
chmod +x scripts/cpp-setup.sh
./scripts/cpp-setup.sh

# Add images to the display directory
cp your-images/* build/bin/assets/images/

# Run the display engine
./scripts/cpp-run.sh
```

**System Requirements for C++:**

- macOS (Apple Silicon or Intel)
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake` or download from cmake.org

### **Option 3: Manual Setup**

If you prefer manual setup or need to modify dependencies:

#### **1. Python Environment**

```bash
cd src/python
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python main.py  # Starts FastAPI server
```

#### **2. C++ Display Engine Setup**

```bash
# One-command C++ setup (downloads openFrameworks, builds everything)
./scripts/cpp-setup.sh

# Run the display engine
./scripts/cpp-run.sh
```

**Alternative manual setup:**

```bash
# Install openFrameworks first
./scripts/setup_openframeworks.sh

# Build and run
mkdir build && cd build
cmake .. && make
cd bin && ./Aetherwave
```

## **📁 Project Layout**

```text
./
├── assets/                 # Your image collections (gitignored)
├── config/                 # Theme configurations and metadata
├── scripts/                # Development and deployment scripts
├── src/
│   ├── cpp/               # openFrameworks rendering engine
│   ├── python/            # Classification API server
│   ├── shared/            # Shared data structures
│   └── ui/                # Optional control panel
├── tests/                 # Test suites
├── docker-compose.yml     # Development environment
└── Dockerfile            # Container configuration
```

## **🔧 Development Commands**

### **Docker Workflow (Recommended)**

```bash
# Start development environment (Python API only)
./scripts/dev-setup.sh

# View logs
docker-compose logs -f classification-api

# Stop all services
./scripts/dev-stop.sh

# Rebuild after code changes
docker-compose build && docker-compose up -d
```

### **C++ Display Engine**

```bash
# One-command setup and build
./scripts/cpp-setup.sh

# Run the display application
./scripts/cpp-run.sh

# Rebuild after C++ changes
./scripts/cpp-setup.sh clean
```

### **Manual Workflow**

```bash
# Python API server
cd src/python && python main.py

# C++ display engine
./scripts/cpp-run.sh

# Run tests
pytest tests/

# Format code
black src/python/ && pylint src/python/
```

## **🧼 Contribution Guidelines**

- See `.github/copilot-instructions.md` for strict coding and contribution standards
- Use `black` and `pylint` (Python), `clang-format` (C++)
- Submit PRs with focused changes, descriptive titles, and test coverage
- Docker setup ensures consistent development environments

## **🔧 Troubleshooting**

### **C++ Display Engine Issues**

**Build fails with openFrameworks errors:**

```bash
# Clean rebuild everything
./scripts/cpp-setup.sh clean

# Or manually clean
rm -rf build libs/openFrameworks
./scripts/cpp-setup.sh
```

**"No images loaded" message:**

- Add images to `build/bin/assets/images/` directory
- Supported formats: JPEG, PNG, GIF, BMP, TIFF
- Images should be in the binary directory, not project root assets

**Application won't start:**

- Check system requirements: `xcode-select --install` and `brew install cmake`
- Verify binary exists: `ls -la build/bin/Aetherwave`
- Run from correct directory: `cd build/bin && ./Aetherwave`

### **Docker/Python API Issues**

**Container fails to start with "unexpected character" or PYTHONPATH errors:**

```bash
# Rebuild containers after recent fixes
docker-compose down
docker-compose build
docker-compose up -d
```

**Port 8000 already in use:**

```bash
# Stop existing services
./scripts/dev-stop.sh
# Or manually stop
docker-compose down
```

**Permission denied on scripts:**

```bash
# Make scripts executable
chmod +x scripts/*.sh
```

**API not accessible from outside container:**

- Fixed: Python API now properly binds to 0.0.0.0 in Docker environment
- The API should be accessible at http://localhost:8000

## **🔒 Privacy and Asset Handling**

- No cloud upload or syncing by default
- All assets remain local
- Docker containers only access mounted asset directories
- No personally identifying info should be embedded

---
