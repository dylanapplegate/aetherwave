"""
Aetherwave Gallery Window - Main display window with multi-monitor support
Handles image display, navigation, and cyberfemme visual effects.
"""

import logging
import random
from typing import Optional, List, Dict, Any
from pathlib import Path

from PySide6.QtWidgets import (
    QMainWindow, QLabel, QVBoxLayout, QWidget, QApplication,
    QHBoxLayout, QFrame, QPushButton, QProgressBar
)
from PySide6.QtCore import (
    Qt, QTimer, QThread, Signal, QSize, QRect, QPropertyAnimation,
    QEasingCurve, Property
)
from PySide6.QtGui import (
    QPixmap, QKeyEvent, QPainter, QPen, QColor, QFont, 
    QLinearGradient, QBrush, QScreen, QPalette
)

from api_client import AetherwaveAPIClient
from config_manager import ConfigManager


class ImageLoader(QThread):
    """Background thread for loading images from the API."""
    
    image_loaded = Signal(str, QPixmap)  # filename, pixmap
    loading_error = Signal(str, str)  # filename, error_message
    
    def __init__(self, api_client: AetherwaveAPIClient):
        super().__init__()
        self.api_client = api_client
        self.pending_loads: List[str] = []
        self.logger = logging.getLogger(__name__)
    
    def load_image(self, filename: str) -> None:
        """Queue an image for loading."""
        if filename not in self.pending_loads:
            self.pending_loads.append(filename)
            if not self.isRunning():
                self.start()
    
    def run(self) -> None:
        """Load queued images using requests (synchronous in thread)."""
        import requests
        
        while self.pending_loads:
            filename = self.pending_loads.pop(0)
            try:
                image_url = self.api_client.get_image_url(filename)
                
                # Use requests for simpler synchronous loading
                response = requests.get(image_url, timeout=10)
                response.raise_for_status()
                
                # Load pixmap from image data
                pixmap = QPixmap()
                if pixmap.loadFromData(response.content):
                    self.image_loaded.emit(filename, pixmap)
                    self.logger.debug(f"Loaded image: {filename}")
                else:
                    self.loading_error.emit(filename, "Failed to decode image data")
                
            except requests.RequestException as e:
                self.loading_error.emit(filename, str(e))
            except Exception as e:
                self.loading_error.emit(filename, str(e))


class CyberfemmeLabel(QLabel):
    """Custom QLabel with cyberfemme visual effects optimized for detailed info display."""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        # Using user's personal brand colors: #FF2D88 (pink), #8A2BE2 (purple), #007BFF (blue)
        self.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 220);
                color: #FFFFFF;
                font-family: 'Perfect DOS VGA 437', 'IBM Plex Mono', 'Courier New', 'PT Mono', 'Source Code Pro', 'Consolas', 'Liberation Mono', 'DejaVu Sans Mono', monospace;
                font-size: 16px;
                font-weight: bold;
                padding: 12px 16px;
                border: 2px solid #FF2D88;
                border-radius: 8px;
                line-height: 1.4;
                max-width: 2000px;
                min-width: 300px;
                white-space: nowrap;
                overflow: hidden;
                text-overflow: ellipsis;
                box-shadow: 0 4px 12px rgba(255, 45, 136, 0.3);
            }
        """)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setWordWrap(False)  # Disable word wrapping to prevent text wrapping
        self.setTextFormat(Qt.RichText)  # Enable HTML/rich text formatting


class GalleryWindow(QMainWindow):
    """Main gallery window with multi-monitor support and cyberfemme aesthetics."""
    
    def __init__(self, api_client: AetherwaveAPIClient, config: ConfigManager):
        super().__init__()
        self.api_client = api_client
        self.config = config
        self.logger = logging.getLogger(__name__)
        
        # Gallery state
        self.image_list: List[str] = []
        self.current_index: int = 0
        self.current_pixmap: Optional[QPixmap] = None
        self.is_playing: bool = False
        
        # Set up UI components with proper typing
        self.image_label: Optional[QLabel] = None
        self.info_label: Optional[QLabel] = None
        self.progress_bar: Optional[QProgressBar] = None
        self.connection_label: Optional[QLabel] = None
        
        # Timers and threads
        self.slideshow_timer: Optional[QTimer] = None
        self.image_loader: Optional[ImageLoader] = None
        self.fade_animation: Optional[QPropertyAnimation] = None
        
        # Multi-monitor support
        self.screens: List[QScreen] = []
        self.primary_screen: Optional[QScreen] = None
        
        self.setup_ui()
        self.setup_multi_monitor()
        self.setup_image_loader()
        self.setup_timers()
        self.load_images()
    
    def setup_ui(self) -> None:
        """Initialize the user interface."""
        # Set window properties
        self.setWindowTitle("Aetherwave Gallery")
        self.setMinimumSize(800, 600)
        
        # Create central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        layout = QVBoxLayout(central_widget)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        
        # Image display area
        self.image_label = QLabel()
        self.image_label.setObjectName("image_label")  # Set object name for CSS selector
        self.image_label.setAlignment(Qt.AlignCenter)
        # Don't set hardcoded background - let adaptive background handle it
        self.image_label.setScaledContents(False)  # We'll handle scaling manually
        layout.addWidget(self.image_label, 1)
        
        # Info overlay
        self.setup_info_overlay()
        
        # Apply cyberfemme styling
        self.apply_cyberfemme_theme()
        
        # Set fullscreen if configured
        if self.config.is_fullscreen():
            self.showFullScreen()
        else:
            self.show()
    
    def setup_info_overlay(self) -> None:
        """Setup the information overlay."""
        # Info label for image details
        self.info_label = CyberfemmeLabel(self)
        if self.info_label is not None:
            self.info_label.setText("Initializing Aetherwave...")
            self.info_label.move(20, 20)
            self.info_label.hide()
        
        # Connection status label
        self.connection_label = CyberfemmeLabel(self)
        if self.connection_label is not None:
            self.connection_label.setText("◉ CONNECTED")
            # Using your brand blue #007BFF for connection status
            self.connection_label.setStyleSheet("""
                QLabel {
                    background-color: rgba(0, 123, 255, 200);
                    color: #FFFFFF;
                    font-size: 12px;
                    font-weight: bold;
                    padding: 8px 12px;
                    border: 2px solid #007BFF;
                    border-radius: 6px;
                    box-shadow: 0 2px 8px rgba(0, 123, 255, 0.4);
                }
            """)
            # Position in bottom left with proper spacing for progress bar
            self.connection_label.move(20, self.height() - 80)
        
        # Progress bar for slideshow
        self.progress_bar = QProgressBar(self)
        if self.progress_bar is not None:
            # Using your brand colors: Pink #FF2D88, Purple #8A2BE2, Blue #007BFF
            self.progress_bar.setStyleSheet("""
                QProgressBar {
                    border: 2px solid #8A2BE2;
                    border-radius: 4px;
                    background-color: rgba(0, 0, 0, 180);
                    color: #FFFFFF;
                    text-align: center;
                    font-size: 11px;
                    font-weight: bold;
                    min-height: 8px;
                    box-shadow: 0 2px 6px rgba(138, 43, 226, 0.3);
                }
                QProgressBar::chunk {
                    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #FF2D88, stop:0.5 #8A2BE2, stop:1 #007BFF);
                    border-radius: 2px;
                }
            """)
            self.progress_bar.setRange(0, 100)
            self.progress_bar.setVisible(False)
    
    def setup_multi_monitor(self) -> None:
        """Setup multi-monitor detection and configuration."""
        app = QApplication.instance()
        self.screens = app.screens()
        self.primary_screen = app.primaryScreen()
        
        screen_count = len(self.screens)
        self.logger.info(f"Detected {screen_count} screen(s)")
        
        for i, screen in enumerate(self.screens):
            geometry = screen.geometry()
            name = screen.name()
            self.logger.info(f"Screen {i}: {name} ({geometry.width()}x{geometry.height()})")
        
        # Position window on primary screen initially
        if self.primary_screen:
            geometry = self.primary_screen.geometry()
            self.move(geometry.topLeft())
            self.resize(geometry.size())
    
    def setup_image_loader(self) -> None:
        """Setup the background image loader."""
        self.image_loader = ImageLoader(self.api_client)
        self.image_loader.image_loaded.connect(self.on_image_loaded)
        self.image_loader.loading_error.connect(self.on_loading_error)
    
    def setup_timers(self) -> None:
        """Setup timers for slideshow and updates."""
        # Slideshow timer
        self.slideshow_timer = QTimer()
        self.slideshow_timer.timeout.connect(self.next_image)
        
        # Update timer for progress bar
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_progress)
        self.update_timer.start(100)  # Update every 100ms
    
    def apply_cyberfemme_theme(self) -> None:
        """Apply cyberfemme visual theme optimized for monochromatic purple/pink/blue artwork."""
        # Set a sophisticated dark background that complements cyberfemme artwork
        # Using deep purple-blue that enhances without competing with the art
        cyberfemme_bg = [25, 20, 35]  # Deep purple-blue background
        
        self.setStyleSheet(f"""
            QMainWindow {{
                background-color: rgb({cyberfemme_bg[0]}, {cyberfemme_bg[1]}, {cyberfemme_bg[2]});
            }}
            QLabel#image_label {{
                background-color: rgb({cyberfemme_bg[0]}, {cyberfemme_bg[1]}, {cyberfemme_bg[2]});
            }}
        """)
        
        self.logger.debug(f"🎭 Applied cyberfemme theme background: rgb({cyberfemme_bg[0]}, {cyberfemme_bg[1]}, {cyberfemme_bg[2]})")
        
        # Set cursor style
        self.setCursor(Qt.BlankCursor if self.config.is_fullscreen() else Qt.ArrowCursor)
    
    def load_images(self) -> None:
        """Load the image list from the API."""
        try:
            self.image_list = self.api_client.get_image_list()
            
            if self.image_list:
                random.shuffle(self.image_list)  # Randomize order
                self.logger.info(f"Loaded {len(self.image_list)} images")
                self.update_info_display()
            else:
                self.show_error("No images found")
                
        except Exception as e:
            self.logger.error(f"Failed to load images: {e}")
            self.show_error(f"Failed to load images: {e}")
    
    def start_gallery(self) -> None:
        """Start the gallery slideshow."""
        if not self.image_list:
            self.show_error("No images to display")
            return
        
        self.is_playing = True
        self.current_index = 0
        self.load_current_image()
        
        # Start slideshow timer
        duration_ms = int(self.config.get_image_duration() * 1000)
        self.slideshow_timer.start(duration_ms)
        
        # Show progress bar
        self.progress_bar.setVisible(True)
        self.position_progress_bar()
        
        self.logger.info("Gallery slideshow started")
    
    def stop_gallery(self) -> None:
        """Stop the gallery slideshow."""
        self.is_playing = False
        if self.slideshow_timer:
            self.slideshow_timer.stop()
        
        self.progress_bar.setVisible(False)
        self.logger.info("Gallery slideshow stopped")
    
    def next_image(self) -> None:
        """Display the next image."""
        if not self.image_list:
            self.logger.debug("🚫 No image list available for next image")
            return
        
        old_index = self.current_index
        self.current_index = (self.current_index + 1) % len(self.image_list)
        new_filename = self.image_list[self.current_index]
        
        self.logger.debug(f"➡️  Navigation: {old_index} → {self.current_index} ({new_filename})")
        
        self.load_current_image()
        self.update_info_display()
    
    def previous_image(self) -> None:
        """Display the previous image."""
        if not self.image_list:
            self.logger.debug("🚫 No image list available for previous image")
            return
        
        old_index = self.current_index
        self.current_index = (self.current_index - 1) % len(self.image_list)
        new_filename = self.image_list[self.current_index]
        
        self.logger.debug(f"⬅️  Navigation: {old_index} → {self.current_index} ({new_filename})")
        
        self.load_current_image()
        self.update_info_display()
    
    def load_current_image(self) -> None:
        """Load the current image from the API."""
        if not self.image_list or self.current_index >= len(self.image_list):
            self.logger.debug(f"🚫 Cannot load image: list={len(self.image_list) if self.image_list else 0}, index={self.current_index}")
            return
        
        filename = self.image_list[self.current_index]
        self.logger.debug(f"📥 Loading image: {filename} (index {self.current_index})")
        self.image_loader.load_image(filename)
    
    def on_image_loaded(self, filename: str, pixmap: QPixmap) -> None:
        """Handle successful image loading."""
        current_filename = self.image_list[self.current_index] if self.image_list else "none"
        self.logger.debug(f"📦 Image loaded: {filename} | Current target: {current_filename} | Match: {filename == current_filename}")
        
        if filename == current_filename:
            self.current_pixmap = pixmap
            pixmap_size = pixmap.size()
            self.logger.debug(f"🖼️  Loaded pixmap size: {pixmap_size.width()}x{pixmap_size.height()}")
            self.display_current_image()
        else:
            self.logger.debug(f"⏭️  Ignoring outdated image load: {filename}")
    
    def on_loading_error(self, filename: str, error_msg: str) -> None:
        """Handle image loading errors."""
        self.logger.warning(f"💥 Failed to load {filename}: {error_msg}")
        # Try next image
        if self.is_playing:
            self.logger.debug("🔄 Auto-advancing to next image due to loading error")
            QTimer.singleShot(1000, self.next_image)
    
    def display_current_image(self) -> None:
        """Display the current image with proper scaling and adaptive background."""
        if not self.current_pixmap:
            self.logger.debug("🚫 No current pixmap available for display")
            return
        
        filename = self.image_list[self.current_index] if self.image_list else "unknown"
        self.logger.debug(f"🖼️  Displaying image: {filename}")
        
        # Scale image to fit window while maintaining aspect ratio
        label_size = self.image_label.size()
        original_size = self.current_pixmap.size()
        
        scaled_pixmap = self.current_pixmap.scaled(
            label_size, 
            Qt.KeepAspectRatio, 
            Qt.SmoothTransformation
        )
        
        scaled_size = scaled_pixmap.size()
        self.logger.debug(f"📐 Image scaling: {original_size.width()}x{original_size.height()} → {scaled_size.width()}x{scaled_size.height()} (label: {label_size.width()}x{label_size.height()})")
        
        self.image_label.setPixmap(scaled_pixmap)
        self.logger.debug("✅ Pixmap set on image label")
        
        # Apply adaptive background color based on image
        self.apply_adaptive_background()
        
        self.logger.debug(f"🎯 Displayed image {self.current_index + 1}/{len(self.image_list)}: {filename}")
    
    def apply_adaptive_background(self) -> None:
        """Apply background color that complements the current image."""
        if not self.image_list:
            self.logger.debug("🚫 No image list available for adaptive background")
            return
            
        try:
            # Get classification data for current image
            filename = self.image_list[self.current_index]
            self.logger.debug(f"🎨 Starting adaptive background for: {filename}")
            
            classification_data = self.api_client.classify_image(filename, include_metadata=True)
            
            if classification_data and 'metadata' in classification_data:
                metadata = classification_data['metadata']
                self.logger.debug(f"📊 Classification metadata keys: {list(metadata.keys())}")
                
                # Get dominant color from classification
                dominant_color = metadata.get('dominant_color')
                self.logger.debug(f"🎯 Raw dominant color: {dominant_color} (type: {type(dominant_color)})")
                
                r, g, b = None, None, None
                
                # Handle both hex string and RGB array formats
                if isinstance(dominant_color, str) and dominant_color.startswith('#'):
                    # Convert hex to RGB
                    hex_color = dominant_color.lstrip('#')
                    if len(hex_color) == 6:
                        r = int(hex_color[0:2], 16)
                        g = int(hex_color[2:4], 16)
                        b = int(hex_color[4:6], 16)
                        self.logger.debug(f"🔄 Converted hex {dominant_color} → RGB({r}, {g}, {b})")
                elif isinstance(dominant_color, list) and len(dominant_color) >= 3:
                    # RGB array format
                    r, g, b = dominant_color[:3]
                    self.logger.debug(f"📋 Using RGB array: ({r}, {g}, {b})")
                
                if r is not None and g is not None and b is not None:
                    # Calculate perceived brightness using luminance formula
                    luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0
                    
                    # Enhanced hue-preserving adaptive background
                    if luminance > 0.7:  # Light images -> light background with hue preservation
                        # For light images, use a brighter version that preserves the hue
                        bg_r = max(220, int(r * 0.85))  # Preserve more of the original color
                        bg_g = max(220, int(g * 0.85))
                        bg_b = max(220, int(b * 0.85))
                        brightness_desc = "light (85%)"
                    elif luminance > 0.4:  # Medium images -> medium background with enhanced hue
                        # For medium images, preserve more hue information
                        bg_r = max(60, int(r * 0.45))
                        bg_g = max(60, int(g * 0.45))
                        bg_b = max(60, int(b * 0.45))
                        brightness_desc = "medium (45%)"
                    else:  # Dark images -> dark background with subtle hue
                        # For dark images, preserve hue but keep it subtle
                        bg_r = max(12, int(r * 0.25))
                        bg_g = max(12, int(g * 0.25))
                        bg_b = max(12, int(b * 0.25))
                        brightness_desc = "dark (25%)"
                    
                    # Calculate hue information for enhanced debugging
                    import colorsys
                    try:
                        h, s, v = colorsys.rgb_to_hsv(r/255.0, g/255.0, b/255.0)
                        hue_degrees = int(h * 360)
                        saturation_pct = int(s * 100)
                        value_pct = int(v * 100)
                        hue_desc = f"H:{hue_degrees}° S:{saturation_pct}% V:{value_pct}%"
                    except:
                        hue_desc = "HSV calculation failed"
                    
                    self.logger.debug(f"✨ Luminance: {luminance:.2f} | {hue_desc} → {brightness_desc} background RGB: ({bg_r}, {bg_g}, {bg_b})")
                    
                    # METHOD 1: Clear existing styles first
                    self.setStyleSheet("")
                    if self.image_label is not None:
                        self.image_label.setStyleSheet("")
                    self.logger.debug("🧹 Cleared existing styles")
                    
                    # METHOD 2: Apply via stylesheet
                    background_style = f"""
                        QMainWindow {{
                            background-color: rgb({bg_r}, {bg_g}, {bg_b});
                        }}
                        QLabel#image_label {{
                            background-color: rgb({bg_r}, {bg_g}, {bg_b});
                        }}
                    """
                    self.setStyleSheet(background_style)
                    self.logger.debug(f"🎨 Applied main window style: rgb({bg_r}, {bg_g}, {bg_b})")
                    
                    # METHOD 3: Set image label directly
                    if self.image_label is not None:
                        label_style = f"background-color: rgb({bg_r}, {bg_g}, {bg_b});"
                        self.image_label.setStyleSheet(label_style)
                        self.logger.debug(f"🏷️  Applied image label style: {label_style}")
                    
                    # METHOD 4: Use Qt palette (more direct approach)
                    try:
                        palette = self.palette()
                        bg_color = QColor(bg_r, bg_g, bg_b)
                        palette.setColor(QPalette.Window, bg_color)
                        palette.setColor(QPalette.Base, bg_color)
                        self.setPalette(palette)
                        
                        # Also set on image label if it exists
                        if self.image_label is not None:
                            label_palette = self.image_label.palette()
                            label_palette.setColor(QPalette.Window, bg_color)
                            label_palette.setColor(QPalette.Base, bg_color)
                            self.image_label.setPalette(label_palette)
                            self.image_label.setAutoFillBackground(True)
                        
                        self.logger.debug(f"🎭 Applied palette colors: {bg_color.name()}")
                    except Exception as palette_error:
                        self.logger.warning(f"⚠️  Palette method failed: {palette_error}")
                    
                    # METHOD 5: Force immediate repaints
                    self.update()
                    if self.image_label is not None:
                        self.image_label.update()
                    self.repaint()
                    if self.image_label is not None:
                        self.image_label.repaint()
                    self.logger.debug("🔄 Forced complete repaint cycle")
                    
                    # Log the final result
                    try:
                        current_bg = self.palette().color(QPalette.Window)
                        self.logger.debug(f"🎯 Final window background: {current_bg.name()}")
                    except Exception as bg_read_error:
                        self.logger.debug(f"🎯 Could not read final background color: {bg_read_error}")
                        
                    self.logger.debug(f"✅ Applied adaptive background: rgb({bg_r}, {bg_g}, {bg_b}) from dominant color {dominant_color}")
                    return  # Successfully applied adaptive background
                else:
                    self.logger.warning(f"❌ Could not extract valid RGB values from dominant color: {dominant_color}")
                    # Fallback to default cyberfemme background
                    self.apply_cyberfemme_theme()
            else:
                self.logger.warning(f"❌ No metadata in classification response for {filename}")
                # Fallback to default cyberfemme background
                self.apply_cyberfemme_theme()
                
        except Exception as e:
            self.logger.warning(f"💥 Failed to apply adaptive background: {e}")
            import traceback
            self.logger.debug(f"🔍 Full traceback: {traceback.format_exc()}")
            # Fallback to default cyberfemme background
            self.apply_cyberfemme_theme()
    
    def update_info_display(self) -> None:
        """Update the information overlay with comprehensive classification data."""
        if not self.image_list or self.info_label is None:
            return
        
        filename = self.image_list[self.current_index]
        
        # Build comprehensive info display
        info_lines = [f"[{self.current_index + 1}/{len(self.image_list)}]"]
        
        try:
            # Get detailed classification data
            classification_data = self.api_client.classify_image(filename, include_metadata=True)
            
            if classification_data and 'metadata' in classification_data:
                metadata = classification_data['metadata']
                
                # Image properties section
                width = metadata.get('width', 0)
                height = metadata.get('height', 0)
                if width and height:
                    aspect_ratio = metadata.get('aspect_ratio', width/height)
                    orientation = "Portrait" if height > width else "Landscape" if width > height else "Square"
                    megapixels = metadata.get('megapixels', (width * height) / 1_000_000)
                    # Handle both numeric and descriptive values
                    try:
                        mp_val = float(megapixels)
                        info_lines.append(f"📐 {width}×{height} :: {orientation} :: {mp_val:.1f}MP")
                    except (ValueError, TypeError):
                        # Use the string value as-is if it can't be converted
                        info_lines.append(f"📐 {width}×{height} :: {orientation} :: {megapixels}MP")
                
                # Color analysis section
                dominant_color = metadata.get('dominant_color', '')
                color_temp = metadata.get('color_temperature', '')
                color_harmony = metadata.get('color_harmony', '')
                if dominant_color:
                    color_info = f"🎨 {dominant_color}"
                    if color_temp:
                        color_info += f" :: {color_temp}"
                    if color_harmony:
                        color_info += f" :: {color_harmony}"
                    info_lines.append(color_info)
                
                # Visual complexity section
                brightness = metadata.get('brightness', 0)
                saturation = metadata.get('saturation', 0)
                edge_density = metadata.get('edge_density', 0)
                if brightness or saturation or edge_density:
                    complexity_info = "📊"
                    if brightness:
                        # Handle both numeric and descriptive values
                        try:
                            bright_val = float(brightness)
                            complexity_info += f" Bright:{bright_val:.1f}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            complexity_info += f" Bright:{brightness}"
                    if saturation:
                        # Handle both numeric and descriptive values
                        try:
                            sat_val = float(saturation)
                            complexity_info += f" Sat:{sat_val:.1f}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            complexity_info += f" Sat:{saturation}"
                    if edge_density:
                        # Handle both numeric and descriptive values
                        try:
                            edge_val = float(edge_density)
                            complexity_info += f" Detail:{edge_val:.1f}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            complexity_info += f" Detail:{edge_density}"
                    info_lines.append(complexity_info)
                
                # Mood and emotion section
                primary_mood = metadata.get('primary_mood', '')
                emotional_tone = metadata.get('emotional_tone', '')
                energy_level = metadata.get('energy_level', '')
                if primary_mood or emotional_tone or energy_level:
                    mood_info = "💫"
                    if primary_mood:
                        mood_info += f" {primary_mood.title()}"
                    if emotional_tone:
                        mood_info += f" :: {emotional_tone.title()}"
                    if energy_level:
                        # Handle both numeric and descriptive values
                        try:
                            energy_val = float(energy_level)
                            mood_info += f" :: Energy:{energy_val:.1f}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            mood_info += f" :: Energy:{energy_level}"
                    info_lines.append(mood_info)
                
                # Technical quality section
                format_type = metadata.get('format', '')
                cinematic_score = metadata.get('cinematic_score', 0)
                classification_confidence = metadata.get('classification_confidence', 0)
                if format_type or cinematic_score or classification_confidence:
                    tech_info = "⚙️"
                    if format_type:
                        tech_info += f" {format_type.upper()}"
                    if cinematic_score:
                        # Handle both numeric and descriptive values
                        try:
                            cinema_val = float(cinematic_score)
                            tech_info += f" :: Cinema:{cinema_val:.1f}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            tech_info += f" :: Cinema:{cinematic_score}"
                    if classification_confidence:
                        # Handle both numeric and descriptive values
                        try:
                            conf_val = float(classification_confidence)
                            tech_info += f" :: Conf:{conf_val:.1%}"
                        except (ValueError, TypeError):
                            # Use the string value as-is if it can't be converted
                            tech_info += f" :: Conf:{classification_confidence}"
                    info_lines.append(tech_info)
            
            # Add theme information if available
            theme_data = self.api_client.get_collection_theme()
            if theme_data and 'theme' in theme_data:
                theme_name = theme_data['theme'].get('theme_name', 'unknown')
                confidence = theme_data['theme'].get('confidence', 0)
                # Handle both numeric and descriptive values
                try:
                    conf_val = float(confidence)
                    info_lines.append(f"🎭 Theme: {theme_name.title()} ({conf_val:.1%})")
                except (ValueError, TypeError):
                    # Use the string value as-is if it can't be converted
                    info_lines.append(f"🎭 Theme: {theme_name.title()} ({confidence})")
                
        except Exception as e:
            # Fallback to basic info if classification fails
            info_lines.append(f"📁 {filename}")
            self.logger.debug(f"Failed to get detailed classification for info display: {e}")
        
        # Join all info lines with newlines and apply alternating brand colors
        # Using your brand colors: #FF2D88 (pink), #8A2BE2 (purple), #007BFF (blue)
        brand_colors = ["#FF2D88", "#8A2BE2", "#007BFF"]
        colored_lines = []
        
        for i, line in enumerate(info_lines):
            color = brand_colors[i % len(brand_colors)]
            colored_lines.append(f'<span style="color: {color};">{line}</span>')
        
        info_text = "<br>".join(colored_lines)
        
        self.info_label.setText(info_text)
        self.info_label.adjustSize()
        
        # Show info for longer to give time to read comprehensive data
        self.info_label.show()
        QTimer.singleShot(8000, self.info_label.hide)  # Show for 8 seconds for more info
    
    def update_progress(self) -> None:
        """Update the slideshow progress bar."""
        if not self.is_playing or not self.slideshow_timer or self.progress_bar is None:
            return
        
        duration_ms = self.slideshow_timer.interval()
        elapsed_ms = duration_ms - self.slideshow_timer.remainingTime()
        progress = int((elapsed_ms / duration_ms) * 100)
        
        self.progress_bar.setValue(progress)
    
    def position_progress_bar(self) -> None:
        """Position the progress bar at the bottom of the window, avoiding connection status."""
        if self.progress_bar is None:
            return
            
        bar_height = 8
        bar_width = self.width() - 40
        x = 20
        # Position above connection status (which is at height - 80) with some padding
        y = self.height() - bar_height - 30
        
        self.progress_bar.setGeometry(x, y, bar_width, bar_height)
    
    def show_connection_error(self) -> None:
        """Show connection error status."""
        if self.connection_label is None:
            return
            
        self.connection_label.setText("◉ DISCONNECTED")
        # Use your brand pink #FF2D88 for error states
        self.connection_label.setStyleSheet("""
            QLabel {
                background-color: rgba(255, 45, 136, 200);
                color: #FFFFFF;
                font-size: 12px;
                font-weight: bold;
                padding: 8px 12px;
                border: 2px solid #FF2D88;
                border-radius: 6px;
                box-shadow: 0 2px 8px rgba(255, 45, 136, 0.4);
            }
        """)
        self.connection_label.show()
    
    def show_error(self, message: str) -> None:
        """Show an error message."""
        if self.info_label is None:
            return
            
        self.info_label.setText(f"ERROR: {message}")
        self.info_label.setStyleSheet("""
            QLabel {
                background-color: rgba(100, 0, 0, 200);
                color: #FF0000;
                border: 2px solid #FF0000;
            }
        """)
        self.info_label.show()
    
    def keyPressEvent(self, event: QKeyEvent) -> None:
        """Handle keyboard input."""
        key = event.key()
        
        if key == Qt.Key_Space or key == Qt.Key_Right:
            self.next_image()
        elif key == Qt.Key_Left or key == Qt.Key_Backspace:
            self.previous_image()
        elif key == Qt.Key_P:
            if self.is_playing:
                self.stop_gallery()
            else:
                self.start_gallery()
        elif key == Qt.Key_F or key == Qt.Key_F11:
            self.toggle_fullscreen()
        elif key == Qt.Key_I:
            # Toggle info display
            if self.info_label is not None:
                if self.info_label.isVisible():
                    self.info_label.hide()
                else:
                    self.update_info_display()
        elif key == Qt.Key_R:
            # Refresh image list
            self.load_images()
        elif key == Qt.Key_Escape or key == Qt.Key_Q:
            self.close()
        else:
            super().keyPressEvent(event)
    
    def toggle_fullscreen(self) -> None:
        """Toggle fullscreen mode."""
        if self.isFullScreen():
            self.showNormal()
            self.setCursor(Qt.ArrowCursor)
        else:
            self.showFullScreen()
            self.setCursor(Qt.BlankCursor)
    
    def resizeEvent(self, event) -> None:
        """Handle window resize events."""
        super().resizeEvent(event)
        
        # Reposition overlays
        if self.connection_label:
            self.connection_label.move(20, self.height() - 60)
        
        if self.progress_bar and self.progress_bar.isVisible():
            self.position_progress_bar()
        
        # Rescale current image
        if self.current_pixmap:
            self.display_current_image()
    
    def closeEvent(self, event) -> None:
        """Handle window close event."""
        self.logger.info("Closing gallery window")
        
        # Stop all timers and threads
        if self.slideshow_timer:
            self.slideshow_timer.stop()
        
        if self.update_timer:
            self.update_timer.stop()
        
        if self.image_loader and self.image_loader.isRunning():
            self.image_loader.quit()
            self.image_loader.wait()
        
        super().closeEvent(event)