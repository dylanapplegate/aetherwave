# Aetherwave Architecture

## Overview

Aetherwave is a sophisticated media display engine designed for high-resolution artwork presentation. The system combines a C++ display engine with a Python classification service to create an intelligent, cinematic viewing experience.

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Aetherwave System                    │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │  C++ Display    │    │   Python Classification    │ │
│  │    Engine       │◄──►│        Service             │ │
│  │                 │    │                             │ │
│  │ • ImageManager  │    │ • Advanced Color Analysis   │ │
│  │ • MonitorMgr    │    │ • Complexity Detection      │ │
│  │ • PerfMonitor   │    │ • Mood Classification       │ │
│  │ • Transitions   │    │ • Batch Processing          │ │
│  └─────────────────┘    └─────────────────────────────┘ │
│           │                         │                   │
│           ▼                         ▼                   │
│  ┌─────────────────┐    ┌─────────────────────────────┐ │
│  │   openFrameworks│    │     FastAPI + OpenCV       │ │
│  │   Rendering     │    │     Analysis Stack          │ │
│  └─────────────────┘    └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Core Components

### 1. C++ Display Engine (`src/cpp/`)

#### ImageManager

- **Purpose**: Advanced image loading, caching, and display management
- **Features**:
  - Asynchronous image loading with preloading
  - Smooth transitions (fade, slide, custom)
  - Multiple display modes (fit, fill, stretch)
  - Support for multiple image formats
- **Key Methods**:
  - `loadImagesFromDirectory()`: Scan and load image collections
  - `startTransition()`: Initiate smooth image transitions
  - `preloadNextImages()`: Background loading for performance

#### MonitorManager

- **Purpose**: Multi-monitor detection and window management
- **Features**:
  - Automatic monitor detection
  - Window positioning and sizing
  - Fullscreen mode management
- **Use Cases**:
  - Gallery installations with multiple displays
  - Presenter mode with control panel

#### PerformanceMonitor

- **Purpose**: Real-time performance tracking and optimization
- **Metrics**:
  - Frame rate monitoring (FPS)
  - Frame time analysis
  - Performance warnings
- **Optimization**:
  - Automatic quality adjustment
  - Performance alerts

### 2. Python Classification Service (`src/python/`)

#### Advanced Color Analysis

- **ColorThief Integration**: Dominant color extraction
- **Color Harmony Analysis**: Complementary, analogous, triadic detection
- **Temperature Analysis**: Warm/cool color classification
- **Perceptual Metrics**: Brightness, saturation, diversity

#### Complexity Analysis

- **Edge Detection**: Canny edge detection for detail analysis
- **Texture Analysis**: Laplacian variance for texture complexity
- **Color Diversity**: Unique color counting and distribution
- **Contrast Measurement**: Standard deviation analysis

#### Mood Classification

- **Multi-factor Analysis**: Color + complexity + composition
- **Mood Categories**: Energetic, serene, dramatic, mysterious, cinematic
- **Confidence Scoring**: Statistical confidence in classification
- **Energy Level**: High/medium/low energy assessment

#### Cinematic Scoring

- **Algorithm**: Weighted combination of visual factors
- **Factors**:
  - Color temperature appropriateness
  - Brightness distribution
  - Mood alignment with cinematic aesthetics
- **Range**: 0.0 (not cinematic) to 1.0 (highly cinematic)

## Data Flow

### Image Processing Pipeline

```
Image File → Basic Info → Color Analysis → Complexity Analysis → Mood Analysis → Metadata
     │            │             │               │                  │             │
     │            ▼             ▼               ▼                  ▼             ▼
     │      [Width/Height]  [Dominant     [Edge Density]    [Primary Mood]  [Complete
     │      [Format/Size]    Colors]      [Texture]         [Energy Level]   Metadata]
     │      [Aspect Ratio]   [Harmony]    [Contrast]        [Confidence]     [JSON]
     │                       [Temp]       [Complexity]      [Tone]
     ▼
Display Engine ← HTTP API ← FastAPI Service ← Classification Results
```

### Real-time Display Loop

```
Frame Start → Update Timers → Check Transitions → Load Images → Draw Scene → UI Overlay → Frame End
     ▲                                                                                        │
     └────────────── Performance Monitoring ← Input Handling ← Event Processing ←──────────┘
```

## Configuration System

### YAML Configuration (`config/config.yaml`)

```yaml
display:
  window:
    width: 1920
    height: 1080
    fullscreen: true
    target_fps: 60
  effects:
    fade_duration: 2.0
    transition_type: "fade"

classification:
  analysis:
    include_colors: true
    include_mood: true
    confidence_threshold: 0.8

assets:
  directories:
    images: "assets/images"
    metadata: "config/metadata"
```

## API Endpoints

### FastAPI Service (Port 8000)

| Endpoint               | Method | Purpose                 | Features                    |
| ---------------------- | ------ | ----------------------- | --------------------------- |
| `/`                    | GET    | Service info            | Version, status, features   |
| `/health`              | GET    | Health check            | System status, dependencies |
| `/classify`            | POST   | Single image analysis   | Full metadata extraction    |
| `/classify/batch`      | POST   | Multiple image analysis | Directory processing        |
| `/metadata/{filename}` | GET    | Cached metadata         | Retrieve stored results     |
| `/analytics/summary`   | GET    | Processing statistics   | Usage analytics             |

### Classification Request/Response

```typescript
// Request
{
  "image_path": "path/to/image.jpg",
  "include_colors": true,
  "include_mood": true,
  "include_complexity": true,
  "save_metadata": true
}

// Response
{
  "ok": true,
  "metadata": {
    "filename": "image.jpg",
    "width": 1920,
    "height": 1080,
    "dominant_color": "#1a1a2e",
    "color_palette": ["#1a1a2e", "#16213e", "#0f3460"],
    "primary_mood": "cinematic",
    "overall_complexity": 0.7,
    "cinematic_score": 0.85,
    "recommended_duration": 12.4
  },
  "processing_time": 0.156
}
```

## Performance Characteristics

### C++ Display Engine

- **Target FPS**: 60 FPS for smooth animations
- **Memory Usage**: Optimized image caching (3-5 images preloaded)
- **Startup Time**: < 2 seconds with image preloading
- **Transition Quality**: Hardware-accelerated smooth transitions

### Python Classification Service

- **Processing Time**: 0.1-0.5 seconds per image (depending on resolution)
- **Memory Usage**: ~100MB base + ~50MB per concurrent classification
- **Throughput**: 10-20 images/second for batch processing
- **Accuracy**: 85-95% confidence for mood classification

## Development Workflow

### Local Development Setup

1. **Environment Setup**

   ```bash
   # Install openFrameworks
   ./scripts/setup_openframeworks.sh

   # Setup Python environment
   python3 -m venv .venv
   source .venv/bin/activate
   pip install -r src/python/requirements.txt
   ```

2. **Build and Run**

   ```bash
   # Start classification service (Terminal 1)
   ./scripts/start_service.sh

   # Start display engine (Terminal 2)
   ./scripts/start_display.sh
   ```

3. **Development Tools**
   - VS Code with C++ and Python extensions
   - FastAPI automatic documentation at `/docs`
   - Performance monitoring with 'P' key
   - Real-time configuration reloading

### Testing Strategy

- **Unit Tests**: Python classification algorithms
- **Integration Tests**: API endpoint functionality
- **Performance Tests**: Frame rate and processing time
- **Visual Tests**: Image transition quality
- **Load Tests**: Batch processing capabilities

## Future Enhancements

### Planned Features

1. **AI Integration**

   - ML-based style classification
   - Content-aware transition selection
   - Automated curation recommendations

2. **Advanced UI**

   - Touch screen support
   - Gesture controls
   - Remote control via web interface

3. **Cloud Integration**

   - Remote image libraries
   - Cloud-based classification
   - Collaborative curation

4. **Installation Features**
   - Museum/gallery mode
   - Scheduled playlists
   - Environmental sensors integration

### Scalability Considerations

- **Horizontal Scaling**: Multiple display nodes
- **Distributed Processing**: Classification service clustering
- **Content Delivery**: CDN integration for large image collections
- **Database Integration**: Metadata persistence and querying

## Security & Privacy

- **Local Processing**: All classification happens locally
- **No Data Collection**: No telemetry or usage tracking
- **File System Safety**: Sandboxed asset directory access
- **Memory Safety**: C++ smart pointers and RAII patterns
