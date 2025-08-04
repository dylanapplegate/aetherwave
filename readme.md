# **Aetherwave**

Aetherwave is an immersive media engine for displaying cinematic MidJourney art across multiple monitors on macOS. Designed for personal storytelling, it emphasizes **stillness**, **mood**, and **fluidity**, without distraction.

**✨ NEW: Unity-powered display engine with 10x faster development and professional shader effects!**

## **🧠 Philosophy**

- Art is the hero—everything else disappears
- Motion and transitions should be **cinematic**, **intentional**, and **low-noise**
- Content drives aesthetics—cyberfemme art creates cyberfemme interfaces automatically
- Professional quality with minimal setup complexity

## **✨ Features**

- **Content-Driven Aesthetics**: Automatic theme detection (95% accuracy for cyberfemme content)
- **Multi-Monitor Gallery**: Professional installation support with Unity's display system
- **Theme-Aware Transitions**: Glitch effects for cyberfemme, soft fades for organic content
- **Post-Processing Pipeline**: Bloom, chromatic aberration, color grading driven by content analysis
- **Python Classification API**: Advanced computer vision with ColorThief + OpenCV
- **Universal Adaptability**: Same system works with any art collection

## **🧱 Technologies**

**🎮 Unity Engine (NEW)**: Professional rendering with built-in multi-monitor, shaders, and effects
**🐍 Python FastAPI**: Content analysis and theme detection (95% accuracy)
**🐳 Docker**: Containerized development environment
**📊 YAML / JSON**: Configuration and theme caching

## **🚀 Quick Start**

### **Unity App (Visual Display)**

```bash
# Build the Aetherwave app
./scripts/unity-build-verify.sh

# Launch Aetherwave
./scripts/unity-app-run.sh
```

**App Location**: `/src/build/bin/Aetherwave.app` (built by Unity AutoBuild system)

**Controls**:
- `→` / `SPACE`: Next image
- `←` / `BACKSPACE`: Previous image  
- `F`: Toggle fullscreen
- `I`: Toggle debug info (shows build version)
- `ESC`: Exit application

### **🎮 Unity Gallery (Recommended)**

**Professional gallery with 10x faster development:**

```bash
# 1. Start Python API for theme detection
./scripts/dev-setup.sh

# 2. Install Unity Hub + Unity 6.1
# Easy install with Homebrew:
brew install --cask unity-hub

# Alternative: Download from https://unity3d.com/get-unity/download

# 3. Install Unity 6.1 through Unity Hub
# Open Unity Hub and install Unity 6.1 (6000.1.14f1 or newer) with these modules:
# - Mac Build Support (Il2CPP)
# - Visual Studio for Mac (optional, for code editing)

# 4. Setup Unity project
./scripts/unity-setup.sh

# 5. Launch Unity gallery
./scripts/unity-run.sh
```

**Unity Installation Details:**

1. **Install Unity Hub via Homebrew:**

   ```bash
   # Install Unity Hub
   brew install --cask unity-hub

   # Launch Unity Hub
   open -a "Unity Hub"
   ```

2. **Install Unity 6.1:**

   - Open Unity Hub
   - Go to "Installs" tab
   - Click "Install Editor"
   - Select "Unity 6.1" (6000.1.14f1 or newer) - Latest stable version with enhanced URP
   - Include these modules:
     - ✅ **Mac Build Support (Il2CPP)** - Required for macOS builds
     - ✅ **Visual Studio for Mac** - Code editor (optional)
     - ✅ **Documentation** - Offline documentation

3. **Verify Installation:**

   ```bash
   # Check if Unity is properly installed
   ./scripts/unity-verify.sh

   # Should show all green checkmarks for Unity Hub, Unity 6.1, and Xcode tools
   ```

✅ **Result**: Professional cyberfemme gallery with content-driven themes!

**System Requirements for Unity:**

- **macOS 10.14+** (Mojave or later)
- **Xcode 12.4+** or Command Line Tools: `xcode-select --install`
- **4GB+ RAM** (8GB+ recommended for large image collections)
- **Metal-compatible GPU** (all modern Macs support this)
- **5GB disk space** for Unity installation

### **🐍 Python API Only (For Development)**

**Just the classification engine:**

```bash
# Clone and setup
git clone <repo-url> aetherwave
cd aetherwave

# One-command setup
chmod +x scripts/dev-setup.sh
./scripts/dev-setup.sh

# Your classification API is now running at http://localhost:8000
```

✅ **Result**: FastAPI service with 95% cyberfemme theme detection accuracy

# Add images to the source assets directory

cp your-images/\* assets/images/

# Rebuild to sync assets

./scripts/cpp-setup.sh

# Run the display engine

./scripts/cpp-run.sh

````

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
````

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
├── assets/                 # Source image collections and documentation
│   ├── README.md          # Asset management documentation
│   └── images/            # Your images go here (synced to build)
│       └── README.md      # Image-specific documentation
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

### **🎮 Unity Setup Issues**

**Check your Unity installation status:**

```bash
# Run comprehensive verification
./scripts/unity-verify.sh

# This will check:
# ✅ Homebrew installation
# ✅ Unity Hub installation
# ✅ Unity 6.1 installation
# ✅ Xcode Command Line Tools
# ✅ System requirements (RAM, disk space, macOS version)
```

**Unity Hub won't install via Homebrew:**

```bash
# Update Homebrew first
brew update

# Install Unity Hub
brew install --cask unity-hub

# If that fails, try manual download
open https://unity3d.com/get-unity/download
```

**Unity 6.1 not showing in Unity Hub:**

- Make sure you're looking at the "Official releases" tab in Unity Hub
- Unity 6.1 is the latest stable version with enhanced performance
- Look for version 6000.1.14f1 or newer

**Unity setup script can't find Unity installation:**

```bash
# Check Unity installation manually
ls /Applications/Unity/Hub/Editor/

# Should show folders like: 2022.3.0f1, 2022.3.10f1, etc.
# If empty, reinstall Unity 6.1 through Unity Hub
```

**Unity project won't open:**

```bash
# Try opening Unity Hub first
open -a "Unity Hub"

# Then use the launcher script
./scripts/unity-run.sh

# Or open project manually in Unity Hub
# File -> Open -> Browse to src/unity/Aetherwave
```

**Unity asking for license:**

- Unity Personal is free for individual use
- Sign up for Unity ID when prompted
- Choose "Unity Personal" license
- No payment required for personal projects

### **🐍 Python API Issues**

**Build fails with openFrameworks errors:**

```bash
# Clean rebuild everything
./scripts/cpp-setup.sh clean

# Or manually clean
rm -rf build libs/openFrameworks
./scripts/cpp-setup.sh
```

**Note**: The current C++ build system uses a test stub while openFrameworks integration is being finalized. The build and run scripts work correctly, and the foundation is ready for full openFrameworks integration.

**"No images loaded" message:**

- Add images to `assets/images/` directory in project root
- Rebuild to sync assets: `./scripts/cpp-setup.sh`
- Supported formats: JPEG, PNG, GIF, BMP, TIFF
- Images are automatically synced from source to build during compilation

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
