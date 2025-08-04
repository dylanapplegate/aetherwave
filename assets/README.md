# Aetherwave Assets Directory

This directory contains the source assets for the Aetherwave display engine. Assets placed here will be automatically synchronized to the build directory during the build process.

## Directory Structure

```
assets/
├── README.md           # This file
└── images/             # Source images for display
    ├── README.md       # Image-specific documentation
    └── [your images]   # Your high-resolution artwork
```

## Usage

### Adding Images

1. Place your images in the `assets/images/` directory
2. Run the build process: `./scripts/cpp-setup.sh`
3. Images will be automatically copied to `build/bin/assets/images/`
4. Launch the application: `./scripts/cpp-run.sh`

### Supported Formats

- **JPEG** (.jpg, .jpeg) - Recommended for photographs
- **PNG** (.png) - Recommended for graphics with transparency
- **GIF** (.gif) - Animated images supported
- **TIFF** (.tiff, .tif) - High-quality uncompressed images
- **BMP** (.bmp) - Basic bitmap format

### File Organization

- **High Resolution**: 4K+ images recommended for multi-monitor displays
- **Naming**: Use descriptive filenames (e.g., `cyberfemme_neon_01.jpg`)
- **Collections**: Organize by theme or aesthetic style
- **Size**: No strict limits, but consider loading performance

## Build Process Integration

The build system automatically:

1. **Scans** the `assets/images/` directory during build
2. **Copies** all supported image formats to `build/bin/assets/images/`
3. **Preserves** directory structure and filenames
4. **Updates** the application's image catalog

## Git Workflow

- ✅ **Tracked**: `assets/README.md` and `assets/images/README.md`
- ❌ **Ignored**: All image files in `assets/images/` (via `.gitignore`)
- 💡 **Reason**: Keeps repository lightweight while preserving asset structure

## Development Tips

- **Testing**: Add a few sample images to verify the pipeline
- **Performance**: Monitor application startup time with large collections
- **Themes**: The content analysis system will automatically detect aesthetic themes
- **Reload**: Use the `r` command in the running application to refresh after adding images

## Integration with Classification System

The Python classification API analyzes images from this directory to:

- Extract dominant color palettes
- Detect mood and aesthetic characteristics
- Generate appropriate UI themes
- Cache analysis results for performance

---

_This directory is part of the Aetherwave content-driven display engine._
