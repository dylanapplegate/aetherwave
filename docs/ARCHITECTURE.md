# Aetherwave Architecture

## Overview

Aetherwave is a sophisticated media display engine designed for high-resolution artwork presentation. The system combines a Qt + PySide6 frontend with a Python FastAPI classification service to create an intelligent, cinematic viewing experience.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Aetherwave System                    │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │  Qt Frontend    │    │   Python Classification    │ │
│  │   (PySide6)     │◄──►│        Service             │ │
│  │                 │    │                             │ │
│  │ • GalleryWindow │    │ • Advanced Color Analysis   │ │
│  │ • ImageLoader   │    │ • Complexity Detection      │ │
│  │ • APIClient     │    │ • Mood Classification       │ │
│  │ • ConfigMgr     │    │ • Batch Processing          │ │
│  └─────────────────┘    └─────────────────────────────┘ │
│           │                         │                   │
│           ▼                         ▼                   │
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │   Qt6 Graphics  │    │     FastAPI + OpenCV       │ │
│  │   & OpenGL      │    │     Analysis Stack          │ │
│  └─────────────────┘    └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Qt Frontend (`src/qt/`)

#### GalleryWindow

- **Purpose**: Main display window with multi-monitor support and cyberfemme aesthetics
- **Features**:
  - Multi-monitor detection and positioning
  - Automatic slideshow with configurable timing
  - Theme-aware UI with magenta/cyan styling
  - Fullscreen mode with hidden cursor
- **Key Methods**:
  - `setup_multi_monitor()`: Detect and configure multiple displays
  - `start_gallery()`: Begin automatic slideshow
  - `next_image()` / `previous_image()`: Manual navigation
  - `toggle_fullscreen()`: Switch display modes

#### ImageLoader

- **Purpose**: Background image loading using QThread
- **Features**:
  - Asynchronous HTTP image fetching
  - Pixmap conversion and scaling
  - Error handling and retry logic
- **Implementation**:
  - Runs in separate thread to prevent UI blocking
  - Uses requests library for reliable HTTP handling
  - Emits signals for loaded images and errors

#### APIClient

- **Purpose**: HTTP interface to FastAPI classification service
- **Features**:
  - Health monitoring and connection status
  - Image list retrieval and caching
  - Theme analysis integration
  - Performance optimization with request caching
- **Key Methods**:
  - `health_check()`: Verify API connectivity
  - `get_image_list()`: Retrieve available images
  - `get_collection_theme()`: Analyze image collection themes

#### ConfigManager

- **Purpose**: YAML-based configuration management
- **Features**:
  - Theme configuration (colors, effects, transparency)
  - Display settings (fullscreen, multi-monitor, timing)
  - Input controls and keyboard shortcuts
- **Configuration Categories**:
  - Display: Fullscreen, FPS, transition settings
  - Effects: Bloom, chromatic aberration, glitch effects
  - Theme: Cyberfemme color palettes and UI styling
  - Input: Keyboard shortcuts and controls

### 2. Python Classification Service (`src/python/`)

#### Content Theme Analyzer

- **Purpose**: Collection-level theme detection from image samples
- **Features**:
  - Statistical analysis across multiple images
  - Theme confidence scoring
  - Color harmony detection
  - Universal adaptability to any art style
- **Algorithms**:
  - Sample random images from collection
  - Extract dominant colors and mood indicators
  - Statistical aggregation for theme determination
  - Confidence scoring based on consistency

#### Advanced Color Analysis

- **ColorThief Integration**: Dominant color extraction with quality optimization
- **Color Harmony Analysis**: Complementary, analogous, triadic detection
- **Temperature Analysis**: Warm/cool color classification
- **Perceptual Metrics**: Brightness, saturation, diversity

#### Mood Classification

- **Multi-factor Analysis**: Color + complexity + composition
- **Mood Categories**: Energetic, serene, dramatic, mysterious, cinematic
- **Confidence Scoring**: Statistical confidence in classification
- **Energy Level**: High/medium/low energy assessment

## Data Flow

### Image Processing Pipeline

```
Image Collection → Sample Selection → Color Analysis → Mood Analysis → Theme Detection
       │                │                  │              │              │
       │                ▼                  ▼              ▼              ▼
       │          [Random Sample]    [Dominant Colors] [Primary Mood] [Cyberfemme/
       │          [Quality Check]    [Color Harmony]  [Energy Level]  Organic/Tech/
       │          [Size Filter]      [Temperature]    [Confidence]    Vintage]
       ▼                                                                │
Qt Frontend ← HTTP Response ← FastAPI Service ← Collection Analysis ←──┘
```

### Real-time Display Loop

```
Frame Update → Check Slideshow → Load Next Image → Apply Scaling → Update UI → Render
     ▲                                                                          │
     └──── Progress Bar ← Theme Display ← Input Handling ← Qt Event Loop ←─────┘
```

## Configuration System

### YAML Configuration (`config/config.yaml`)

```yaml
display:
  fullscreen: true
  multi_monitor: true
  vsync: true
  target_fps: 60
  transition_duration: 1.0
  image_duration: 5.0

effects:
  bloom_enabled: true
  bloom_intensity: 0.3
  chromatic_aberration: true
  aberration_strength: 0.02
  glitch_enabled: true
  scan_lines: true

theme:
  cyberfemme:
    primary_color: [255, 0, 255]    # Magenta
    secondary_color: [0, 255, 255]  # Cyan
    accent_color: [255, 255, 255]   # White
    ui_transparency: 0.8

input:
  space_next_image: true
  arrow_navigation: true
  escape_exit: true
  f_fullscreen_toggle: true
```

## API Endpoints

### FastAPI Service (Port 8000)

| Endpoint                    | Method | Purpose                 | Features                    |
| --------------------------- | ------ | ----------------------- | --------------------------- |
| `/health`                   | GET    | Health check            | System status, dependencies |
| `/images/list`              | GET    | Available images        | Filename list, count        |
| `/images/{filename}`        | GET    | Serve image file        | HTTP image serving          |
| `/classify`                 | POST   | Single image analysis   | Full metadata extraction    |
| `/analyze/collection-theme` | POST   | Collection theme analysis | Theme detection, confidence |

### Collection Theme Analysis

```typescript
// Request
{
  "collection_path": "assets/images",
  "sample_size": 5
}

// Response
{
  "theme": {
    "theme_name": "cyberfemme",
    "confidence": 0.95,
    "primary_colors": ["#FF00FF", "#00FFFF", "#FFFFFF"],
    "temperature": "cool",
    "mood": "mysterious",
    "energy_level": "high"
  },
  "collection_stats": {
    "total_images": 82,
    "analyzed_images": 5,
    "processing_time": 0.023
  }
}
```

## Performance Characteristics

### Qt Frontend

- **Target FPS**: 60 FPS with VSync for smooth animations
- **Memory Usage**: Efficient with single-image loading
- **Startup Time**: < 1 second with API connectivity
- **Multi-Monitor**: Native Qt6 support for seamless display coordination

### Python Classification Service

- **Processing Time**: 
  - Individual images: 0.4-0.8 seconds (1.5-6.7MB PNG files)
  - Collection analysis: 0.023 seconds (5 images)
- **Memory Usage**: ~100MB base + efficient ColorThief quality=10 settings
- **Accuracy**: 95% confidence for cyberfemme theme detection
- **Optimization**: Quality=10 with image preprocessing for large files

## Development Workflow

### Local Development Setup

1. **Environment Setup**

   ```bash
   # Install Qt dependencies
   pip install PySide6 PyYAML requests Pillow

   # Setup Python environment
   python3 -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
   ```

2. **Run Application**

   ```bash
   # Start classification service (Docker)
   ./scripts/dev-setup.sh

   # Start Qt gallery
   ./scripts/qt-run.sh
   
   # Or manually
   cd src/qt && python main.py
   ```

3. **Development Tools**
   - VS Code with Python extensions
   - FastAPI automatic documentation at `/docs`
   - Qt Designer for UI modifications
   - Real-time configuration with YAML hot reload

### Testing Strategy

- **Unit Tests**: Python classification algorithms with pytest
- **Integration Tests**: Qt-FastAPI communication
- **Performance Tests**: Image loading and display timing
- **Visual Tests**: Theme accuracy and UI rendering
- **Multi-Monitor Tests**: Display coordination across screens

## Multi-Monitor Support

### Qt6 Display Management

- **Automatic Detection**: QApplication.screens() for monitor enumeration
- **Display Properties**: Resolution, DPI, position detection
- **Window Positioning**: Intelligent placement across displays
- **Fullscreen Coordination**: Seamless fullscreen across monitors

### Gallery Installation Mode

- **Professional Setup**: Gallery installations with multiple displays
- **Content Distribution**: Single collection across multiple screens
- **Synchronized Display**: Coordinated image transitions
- **Remote Control**: Keyboard controls work across all displays

## Theme System

### Content-Driven Aesthetics

- **Cyberfemme**: Magenta/cyan UI for purple/pink artwork
- **Organic**: Earth tones for natural content
- **Tech**: Minimal design for technical/architectural content
- **Adaptive**: Universal fallback with neutral styling

### Visual Effects Pipeline

- **UI Transparency**: Configurable transparency for overlays
- **Color Coordination**: UI colors match detected content themes
- **Progress Indicators**: Theme-aware progress bars and status
- **Typography**: Monospace fonts for cyberpunk aesthetic

## Security & Privacy

- **Local Processing**: All classification happens locally
- **No Data Collection**: No telemetry or usage tracking
- **File System Safety**: Read-only asset directory access
- **HTTP Security**: Local API communication only

## Future Enhancements

### Planned Features

1. **OpenGL Effects Pipeline**
   - Bloom and chromatic aberration shaders
   - Glitch effects for cyberfemme themes
   - Hardware-accelerated transitions

2. **Advanced Multi-Monitor**
   - Content distribution strategies
   - Independent display control
   - Synchronized playlist management

3. **Installation Features**
   - Kiosk mode with auto-start
   - Crash recovery and auto-restart
   - Environmental sensor integration

### Scalability Considerations

- **Performance Optimization**: Image caching and preloading
- **Memory Management**: Efficient Qt6 resource handling
- **Network Optimization**: HTTP connection pooling
- **Collection Scaling**: Support for thousands of images