# Unity Setup for Aetherwave

This guide will help you set up the Unity-based Aetherwave gallery system.

## Quick Start (Recommended)

1. **Install Unity Hub & Unity 2022.3 LTS**
   ```bash
   # Download Unity Hub from: https://unity3d.com/get-unity/download
   # Install Unity 2022.3 LTS through Unity Hub
   ```

2. **Run Unity Setup Script**
   ```bash
   ./scripts/unity-setup.sh
   ```

3. **Launch Unity Project**
   ```bash
   ./scripts/unity-run.sh
   ```

## Prerequisites

### Unity Installation
- **Unity Hub**: Download from [Unity Download Page](https://unity3d.com/get-unity/download)
- **Unity 2022.3 LTS**: Install through Unity Hub
- **Include Components**:
  - Mac Build Support (for macOS)
  - Visual Studio for Mac (optional, for code editing)

### Python API (Required)
The Unity gallery connects to the Python classification API for theme-driven aesthetics:

```bash
# Start Python API
./scripts/dev-setup.sh

# Verify API is running
curl http://localhost:8000/health
```

## Project Structure

```
src/unity/Aetherwave/
├── Assets/
│   ├── Scenes/          # Unity scenes
│   ├── Scripts/         # C# scripts
│   │   ├── AetherwaveGallery.cs          # Main gallery controller
│   │   └── Managers/
│   │       ├── ImageDisplayManager.cs     # Image loading & transitions
│   │       ├── ThemeManager.cs           # Theme-driven post-processing
│   │       ├── PythonAPIClient.cs        # HTTP client for Python API
│   │       └── MultiMonitorController.cs # Multi-display support
│   ├── Materials/       # Unity materials
│   ├── Shaders/         # Custom shaders (future)
│   └── StreamingAssets/ # Runtime assets
│       └── Images/      # Your artwork goes here
└── ProjectSettings/     # Unity project configuration
```

## Setting Up Your Images

1. **Copy images to StreamingAssets**:
   ```bash
   cp -r assets/images/* src/unity/Aetherwave/Assets/StreamingAssets/Images/
   ```

2. **Supported formats**: JPG, JPEG, PNG, BMP, TGA

## Unity Project Configuration

### Universal Render Pipeline (URP)
The project uses Unity's Universal Render Pipeline for:
- ✅ Post-processing effects (bloom, chromatic aberration)
- ✅ Multi-monitor rendering
- ✅ Hardware-accelerated graphics

### Post-Processing Volume
Theme-driven visual effects:
- **Cyberfemme**: High bloom + chromatic aberration + enhanced saturation
- **Organic**: Soft bloom + warm vignette + natural tones
- **Tech**: High contrast + minimal effects + cool tones
- **Vintage**: No bloom + sepia vignette + desaturated

## Controls

| Input | Action |
|-------|--------|
| `Space` / `→` / `↓` | Next image |
| `Backspace` / `←` / `↑` | Previous image |
| `T` | Refresh theme from Python API |
| `F1` | Toggle debug information |
| `ESC` | Exit gallery |

## Python API Integration

The Unity gallery communicates with your Python FastAPI service:

### API Endpoints Used:
- `GET /health` - Verify API connection
- `GET /analyze/collection-theme` - Get current theme profile

### Theme Data Structure:
```json
{
  "theme_profile": {
    "themeName": "cyberfemme",
    "confidence": 0.91,
    "primaryColor": {"r": 0.8, "g": 0.2, "b": 0.8},
    "accentColor": {"r": 0.0, "g": 1.0, "b": 1.0},
    "backgroundColor": {"r": 0.1, "g": 0.1, "b": 0.2},
    "transitionDuration": 1.2,
    "effectIntensity": 0.8
  }
}
```

## Development Workflow

### 1. Start Python API
```bash
./scripts/dev-setup.sh
```

### 2. Open Unity Project
```bash
./scripts/unity-run.sh
```

### 3. Create Main Scene
1. Create new scene: `Gallery`
2. Add `AetherwaveGallery` script to main GameObject
3. Set up UI Canvas with RawImage for display
4. Assign components in inspector

### 4. Build & Test
- Use Unity's Play mode for testing
- Build standalone for deployment

## Multi-Monitor Support

Unity provides built-in multi-monitor support:

1. **Automatic Detection**: Displays detected on startup
2. **Layout Modes**:
   - Primary: Single display only
   - Span: Span across all displays  
   - Duplicate: Same content on all displays
   - Gallery: Custom layout per display

## Troubleshooting

### Unity Won't Start
- Verify Unity 2022.3 LTS is installed
- Check Unity Hub is running
- Ensure project path is correct

### No Images Loading
- Check `StreamingAssets/Images/` folder exists
- Verify image file formats are supported
- Check Unity Console for loading errors

### Python API Connection Failed
- Ensure Python API is running (`./scripts/dev-setup.sh`)
- Check API health: `curl http://localhost:8000/health`
- Verify no firewall blocking localhost:8000

### Theme Effects Not Working
- Check Post Processing Volume is assigned
- Verify URP is configured correctly
- Look for theme data in Unity Console

## Performance Optimization

### For Large Collections:
- Disable `enableImagePreloading` for collections >100 images
- Reduce `maxTextureSize` for 4K+ images
- Use selective loading for better memory management

### For 4K+ Displays:
- Ensure hardware acceleration is enabled
- Monitor GPU memory usage
- Consider texture compression

## Next Steps

1. **Create Main Scene**: Set up UI and assign script components
2. **Configure Post-Processing**: Set up URP volume for theme effects
3. **Test Integration**: Verify Python API communication
4. **Add Shader Effects**: Custom cyberfemme shaders (Phase 4B.3)
5. **Multi-Monitor Setup**: Configure displays for gallery installation

## Migration Benefits

Unity provides significant advantages over the SDL2 implementation:

- ✅ **10x faster development** with built-in systems
- ✅ **Professional post-processing** pipeline
- ✅ **Multi-monitor support** without manual coordination
- ✅ **Hardware acceleration** guaranteed
- ✅ **VR/AR readiness** for future features
- ✅ **Cross-platform deployment**

Your proven Python API remains unchanged, providing the same 95% theme detection accuracy!