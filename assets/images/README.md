# Aetherwave Image Assets

Place your high-resolution artwork in this directory for display in the Aetherwave engine.

## Quick Start
1. **Add images** to this directory
2. **Run build**: `./scripts/cpp-setup.sh` (from project root)
3. **Launch app**: `./scripts/cpp-run.sh`

## Supported Formats
- **JPEG** (.jpg, .jpeg) - Best for photographs and complex artwork
- **PNG** (.png) - Best for graphics with transparency or text
- **GIF** (.gif) - Animated images and simple graphics
- **TIFF** (.tiff, .tif) - Highest quality, uncompressed images
- **BMP** (.bmp) - Basic bitmap format

## Recommendations

### Image Quality
- **Resolution**: 4K+ (3840×2160) recommended for multi-monitor setups
- **Aspect Ratio**: 16:9 or 21:9 work best for widescreen displays
- **Color Depth**: 24-bit or higher for rich color reproduction

### File Organization
- Use descriptive filenames: `neon_cityscape_01.jpg`
- Group similar themes or styles together
- Consider file sizes for smooth transitions

### Performance Notes
- **Loading Time**: Larger files take longer to load and analyze
- **Memory Usage**: High-resolution images use more system memory
- **Analysis Speed**: Complex images take longer for theme detection

## Content-Driven Aesthetics

The Aetherwave engine automatically analyzes your images to:
- **Extract** dominant color palettes
- **Detect** mood and aesthetic characteristics
- **Generate** matching UI themes and transitions
- **Adapt** the interface to complement your artwork

No manual configuration required - the system learns from your content!

## Build Integration

During the build process (`./scripts/cpp-setup.sh`):
1. Images are **copied** from here to `build/bin/assets/images/`
2. The application **scans** for supported formats
3. **Metadata** is generated for theme analysis
4. **Cache files** are created for faster subsequent loads

## Live Reload

While the application is running:
- Add new images to this directory
- Type `r` in the application to reload
- New images will be automatically detected and added

---
*Add your MidJourney artwork, digital art, or any high-resolution images here to create stunning visual displays.*
