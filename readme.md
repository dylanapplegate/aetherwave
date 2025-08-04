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

### **Option 2: Manual Setup**

If you prefer manual setup or need to modify dependencies:

#### **1. Python Environment**

```bash
cd src/python
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python main.py  # Starts FastAPI server
```

#### **2. C++ / openFrameworks Setup**

```bash
# Follow openFrameworks installation guide for macOS
# Clone this repo into apps/myApps/ folder
# Use projectGenerator to create .xcodeproj
cd src/cpp
make run  # or open in Xcode
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
# Start development environment
./scripts/dev-setup.sh

# View logs
docker-compose logs -f classification-api

# Stop all services
./scripts/dev-stop.sh

# Rebuild after code changes
docker-compose build && docker-compose up -d
```

### **Manual Workflow**

```bash
# Python API server
cd src/python && python main.py

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

## **🔒 Privacy and Asset Handling**

- No cloud upload or syncing by default
- All assets remain local
- Docker containers only access mounted asset directories
- No personally identifying info should be embedded

---
