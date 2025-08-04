# Aetherwave Setup Guide

## Prerequisites

### macOS (Apple Silicon)

1. **Xcode Command Line Tools**

   ```bash
   xcode-select --install
   ```

2. **Homebrew**

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **CMake and Build Tools**

   ```bash
   brew install cmake pkg-config
   ```

4. **Python 3.11+**
   ```bash
   brew install python@3.11
   ```

## openFrameworks Installation

### Automatic Installation (Recommended)

Run the provided setup script:

```bash
./scripts/setup_openframeworks.sh
```

### Manual Installation

1. **Download openFrameworks**

   ```bash
   mkdir -p libs
   cd libs
   curl -L "https://github.com/openframeworks/openFrameworks/releases/download/0.12.0/of_v0.12.0_osx_release.tar.gz" | tar xz
   mv of_v0.12.0_osx_release openFrameworks
   ```

2. **Compile openFrameworks**

   ```bash
   cd openFrameworks/scripts/osx
   ./compileOF.sh
   ```

3. **Install Dependencies**
   ```bash
   cd ../../../openFrameworks/scripts/osx
   ./install_dependencies.sh
   ```

## Python Environment Setup

1. **Create Virtual Environment**

   ```bash
   python3.11 -m venv .venv
   source .venv/bin/activate
   ```

2. **Install Dependencies**

   ```bash
   pip install -r requirements.txt
   ```

3. **Verify Installation**
   ```bash
   python check_environment.py
   ```

## Build Configuration

1. **Configure CMake**

   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

2. **Build Application**
   ```bash
   make -j$(sysctl -n hw.ncpu)
   ```

## Running Aetherwave

### C++ Application

```bash
cd build/bin
./Aetherwave
```

### Python Classification Service

```bash
source .venv/bin/activate
cd src/python
python main.py
```

### Full System

```bash
# Terminal 1: Start classification service
./scripts/start_service.sh

# Terminal 2: Run display engine
./scripts/start_display.sh
```

## Troubleshooting

### Common Issues

1. **openFrameworks not found**

   - Ensure `OF_ROOT` environment variable is set
   - Check that openFrameworks is compiled correctly

2. **Python import errors**

   - Verify virtual environment is activated
   - Check Python path in VS Code settings

3. **Build failures**
   - Update Xcode command line tools
   - Clear CMake cache: `rm -rf build/CMakeCache.txt`

### Performance Optimization

1. **For High-Resolution Displays**

   ```yaml
   display:
     window:
       width: 3840
       height: 2160
       target_fps: 30 # Reduce for 4K
   ```

2. **For Multiple Monitors**
   ```yaml
   display:
     monitors:
       primary: 0
       extended: [1, 2]
   ```

## Development Workflow

1. **Code Changes**: Edit source files
2. **Build**: `make` in build directory
3. **Test**: Run automated tests
4. **Deploy**: Copy to target system

For detailed architecture information, see [ARCHITECTURE.md](ARCHITECTURE.md).
