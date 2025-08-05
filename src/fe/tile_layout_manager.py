"""
Aetherwave Tile Layout Manager - Bento box style shifting tiles layout
Manages dynamic multi-image arrangements with varied tile sizes and smooth animations.
"""

import logging
import random
import math
from typing import List, Dict, Tuple, Optional, Any
from dataclasses import dataclass
from enum import Enum

from PySide6.QtWidgets import (
    QWidget, QLabel, QGridLayout, QVBoxLayout,
    QGraphicsOpacityEffect, QMainWindow
)
from PySide6.QtCore import Qt, QTimer, QSize, QPropertyAnimation, QParallelAnimationGroup, QEasingCurve, QObject, Signal, QRect
from PySide6.QtGui import QPixmap, QPainter, QLinearGradient, QColor, QPalette, QBrush

from api_client import AetherwaveAPIClient


class TileSize(Enum):
    """Predefined tile sizes for bento box layout."""
    SMALL = (1, 1)      # 1x1 grid cell
    MEDIUM = (2, 1)     # 2x1 grid cell
    LARGE = (2, 2)      # 2x2 grid cell
    WIDE = (3, 1)       # 3x1 grid cell
    TALL = (1, 2)       # 1x2 grid cell
    XLARGE = (3, 2)     # 3x2 grid cell


@dataclass
class TileSpec:
    """Specification for a single tile in the layout."""
    row: int
    col: int
    width: int  # grid cells
    height: int  # grid cells
    size_type: TileSize
    image_filename: Optional[str] = None


@dataclass
class BentoPattern:
    """Predefined bento box layout pattern."""
    name: str
    grid_cols: int
    grid_rows: int
    tiles: List[TileSpec]

    @property
    def total_tiles(self) -> int:
        return len(self.tiles)


class TileWidget(QLabel):
    """Individual tile widget for displaying images with animations."""

    def __init__(self, tile_spec: TileSpec, parent=None):
        super().__init__(parent)
        self.tile_spec = tile_spec
        self.logger = logging.getLogger(__name__)

        # Setup widget properties for complete fill
        self.setAlignment(Qt.AlignCenter)
        self.setScaledContents(True)  # Scale content to fill completely
        self.setMinimumSize(50, 50)

        # Remove all margins and padding
        self.setContentsMargins(0, 0, 0, 0)

        # Animation properties
        self.opacity_effect = QGraphicsOpacityEffect()
        self.setGraphicsEffect(self.opacity_effect)
        self.opacity_effect.setOpacity(0.0)  # Start invisible

        # Styling - remove borders and background to show only image
        self.setStyleSheet("""
            TileWidget {
                border: none;
                border-radius: 0px;
                background-color: transparent;
                margin: 0px;
                padding: 0px;
            }
        """)

    def set_image(self, pixmap: QPixmap) -> None:
        """Set the image for this tile with intelligent cropping - NO STRETCHING EVER."""
        if pixmap.isNull():
            return

        # Get tile size
        tile_size = self.size()
        if tile_size.width() <= 0 or tile_size.height() <= 0:
            # Use minimum size if actual size not available yet
            tile_size = QSize(200, 200)

        # Scale image maintaining aspect ratio with intelligent cropping
        scaled_pixmap = self._scale_with_smart_crop(pixmap, tile_size, crop_factor=0.0)

        # Set the pixmap directly on this QLabel
        # CRITICAL: setScaledContents(False) ensures no stretching/distortion
        self.setScaledContents(False)
        self.setPixmap(scaled_pixmap)

        # Store for animations
        self.current_pixmap = scaled_pixmap

    def _scale_with_smart_crop(self, pixmap: QPixmap, target_size: QSize, crop_factor: float = 0.2) -> QPixmap:
        """Scale image with intelligent cropping to fill target while maintaining aspect ratio."""
        if pixmap.isNull():
            return pixmap

        # Get original dimensions
        orig_width = pixmap.width()
        orig_height = pixmap.height()
        target_width = target_size.width()
        target_height = target_size.height()

        # Calculate aspect ratios
        orig_ratio = orig_width / orig_height
        target_ratio = target_width / target_height

        # ALWAYS maintain aspect ratio - scale to fill and crop excess
        if orig_ratio > target_ratio:
            # Image is wider than target - scale by height, crop width
            scale_factor = target_height / orig_height
            scaled_width = int(orig_width * scale_factor)
            scaled_height = target_height
        else:
            # Image is taller than target - scale by width, crop height
            scale_factor = target_width / orig_width
            scaled_width = target_width
            scaled_height = int(orig_height * scale_factor)

        # Scale the image maintaining aspect ratio
        scaled_pixmap = pixmap.scaled(
            scaled_width, scaled_height,
            Qt.AspectRatioMode.KeepAspectRatio,  # NEVER distort!
            Qt.TransformationMode.SmoothTransformation
        )

        # Center crop to exact target size if needed
        if scaled_width > target_width or scaled_height > target_height:
            # Calculate center crop position
            crop_x = max(0, (scaled_width - target_width) // 2)
            crop_y = max(0, (scaled_height - target_height) // 2)

            # Ensure we don't exceed bounds
            actual_width = min(target_width, scaled_width)
            actual_height = min(target_height, scaled_height)

            # Crop to exact target size
            cropped_pixmap = scaled_pixmap.copy(crop_x, crop_y, actual_width, actual_height)

            # If cropped result is smaller than target, create a new pixmap and center the image
            if actual_width < target_width or actual_height < target_height:
                result_pixmap = QPixmap(target_width, target_height)
                result_pixmap.fill(Qt.GlobalColor.black)  # Fill with black background

                painter = QPainter(result_pixmap)
                painter.setRenderHint(QPainter.RenderHint.Antialiasing)

                # Center the cropped image
                x_offset = (target_width - actual_width) // 2
                y_offset = (target_height - actual_height) // 2

                painter.drawPixmap(x_offset, y_offset, cropped_pixmap)
                painter.end()

                return result_pixmap

            return cropped_pixmap

        return scaled_pixmap

    def resizeEvent(self, event) -> None:
        """Handle resize events and apply pending pixmap."""
        super().resizeEvent(event)
        if hasattr(self, '_pending_pixmap') and self._pending_pixmap:
            pixmap = self._pending_pixmap
            self._pending_pixmap = None
            self.set_image(pixmap)

    def fade_in(self, duration: int = 800) -> None:
        """Animate tile with drop-in effect and fade."""
        # Start from scaled down and transparent
        self.opacity_effect.setOpacity(0.0)

        # Create property animations
        self.fade_animation = QPropertyAnimation(self.opacity_effect, b"opacity")
        self.fade_animation.setDuration(duration)
        self.fade_animation.setStartValue(0.0)
        self.fade_animation.setEndValue(1.0)
        self.fade_animation.setEasingCurve(QEasingCurve.OutBack)  # Bounce effect

        # Scale animation for drop-in effect
        self.scale_animation = QPropertyAnimation(self, b"geometry")
        current_geometry = self.geometry()

        # Start smaller and centered
        start_geometry = QRect(
            current_geometry.center().x() - current_geometry.width()//4,
            current_geometry.center().y() - current_geometry.height()//4,
            current_geometry.width()//2,
            current_geometry.height()//2
        )

        self.scale_animation.setDuration(duration)
        self.scale_animation.setStartValue(start_geometry)
        self.scale_animation.setEndValue(current_geometry)
        self.scale_animation.setEasingCurve(QEasingCurve.OutBack)

        # Run animations in parallel
        self.animation_group = QParallelAnimationGroup()
        self.animation_group.addAnimation(self.fade_animation)
        self.animation_group.addAnimation(self.scale_animation)
        self.animation_group.start()

    def fade_out(self, duration: int = 400) -> None:
        """Animate tile fade out with scale effect."""
        self.fade_animation = QPropertyAnimation(self.opacity_effect, b"opacity")
        self.fade_animation.setDuration(duration)
        self.fade_animation.setStartValue(1.0)
        self.fade_animation.setEndValue(0.0)
        self.fade_animation.setEasingCurve(QEasingCurve.InCubic)

        # Scale down while fading
        self.scale_animation = QPropertyAnimation(self, b"geometry")
        current_geometry = self.geometry()
        end_geometry = QRect(
            current_geometry.center().x() - current_geometry.width()//6,
            current_geometry.center().y() - current_geometry.height()//6,
            current_geometry.width()//3,
            current_geometry.height()//3
        )

        self.scale_animation.setDuration(duration)
        self.scale_animation.setStartValue(current_geometry)
        self.scale_animation.setEndValue(end_geometry)
        self.scale_animation.setEasingCurve(QEasingCurve.InCubic)

        # Run animations in parallel
        self.animation_group = QParallelAnimationGroup()
        self.animation_group.addAnimation(self.fade_animation)
        self.animation_group.addAnimation(self.scale_animation)
        self.animation_group.start()


class TileLayoutManager(QObject):
    """Manages bento box style tile layouts with smooth transitions."""

    layout_changed = Signal()  # Emitted when layout changes

    def __init__(self, parent_widget: QWidget, api_client: AetherwaveAPIClient):
        super().__init__()
        self.parent_widget = parent_widget
        self.api_client = api_client
        self.logger = logging.getLogger(__name__)

        # Layout state
        self.current_pattern: Optional[BentoPattern] = None
        self.tiles: List[TileWidget] = []
        self.grid_layout: Optional[QGridLayout] = None
        self.image_cache: Dict[str, QPixmap] = {}

        # Animation timers
        self.shift_timer = QTimer()
        self.shift_timer.timeout.connect(self.shift_tiles)
        self.shift_timer.setSingleShot(False)

        # Layout configuration
        self.gutter_size = 12  # Space between tiles
        self.base_tile_size = 150  # Base size for 1x1 tile
        self.shift_interval = 8000  # 8 seconds between shifts

        # Create predefined bento patterns
        self.patterns = self._create_bento_patterns()

        self.logger.info("Tile layout manager initialized")

    def _create_bento_patterns(self) -> List[BentoPattern]:
        """Create various bento box layout patterns."""
        patterns = []

        # Pattern 1: Mixed sizes (4x3 grid)
        patterns.append(BentoPattern(
            name="mixed_classic",
            grid_cols=4,
            grid_rows=3,
            tiles=[
                TileSpec(0, 0, 2, 2, TileSize.LARGE),     # Top left large
                TileSpec(0, 2, 1, 1, TileSize.SMALL),     # Top middle small
                TileSpec(0, 3, 1, 1, TileSize.SMALL),     # Top right small
                TileSpec(1, 2, 2, 1, TileSize.MEDIUM),    # Middle right medium
                TileSpec(2, 0, 1, 1, TileSize.SMALL),     # Bottom left small
                TileSpec(2, 1, 1, 1, TileSize.SMALL),     # Bottom middle small
                TileSpec(2, 2, 2, 1, TileSize.MEDIUM),    # Bottom right medium
            ]
        ))

        # Pattern 2: Tall focus (3x4 grid)
        patterns.append(BentoPattern(
            name="tall_focus",
            grid_cols=3,
            grid_rows=4,
            tiles=[
                TileSpec(0, 0, 1, 2, TileSize.TALL),      # Left tall
                TileSpec(0, 1, 2, 2, TileSize.LARGE),     # Center large
                TileSpec(0, 2, 1, 1, TileSize.SMALL),     # Top right small
                TileSpec(1, 2, 1, 1, TileSize.SMALL),     # Middle right small
                TileSpec(2, 0, 1, 1, TileSize.SMALL),     # Bottom left small
                TileSpec(2, 1, 2, 1, TileSize.MEDIUM),    # Bottom center medium
                TileSpec(3, 0, 1, 1, TileSize.SMALL),     # Bottom left small
                TileSpec(3, 1, 2, 1, TileSize.MEDIUM),    # Bottom span medium
            ]
        ))

        # Pattern 3: Wide showcase (5x3 grid)
        patterns.append(BentoPattern(
            name="wide_showcase",
            grid_cols=5,
            grid_rows=3,
            tiles=[
                TileSpec(0, 0, 3, 2, TileSize.XLARGE),    # Large showcase
                TileSpec(0, 3, 1, 1, TileSize.SMALL),     # Top right small
                TileSpec(0, 4, 1, 1, TileSize.SMALL),     # Top far right small
                TileSpec(1, 3, 2, 1, TileSize.MEDIUM),    # Middle right medium
                TileSpec(2, 0, 1, 1, TileSize.SMALL),     # Bottom left small
                TileSpec(2, 1, 2, 1, TileSize.MEDIUM),    # Bottom center medium
                TileSpec(2, 3, 2, 1, TileSize.MEDIUM),    # Bottom right medium
            ]
        ))

        # Pattern 4: Grid balanced (4x4 grid)
        patterns.append(BentoPattern(
            name="grid_balanced",
            grid_cols=4,
            grid_rows=4,
            tiles=[
                TileSpec(0, 0, 2, 2, TileSize.LARGE),     # Top left large
                TileSpec(0, 2, 1, 1, TileSize.SMALL),     # Top middle small
                TileSpec(0, 3, 1, 2, TileSize.TALL),      # Top right tall
                TileSpec(1, 2, 1, 1, TileSize.SMALL),     # Middle small
                TileSpec(2, 0, 1, 1, TileSize.SMALL),     # Bottom left small
                TileSpec(2, 1, 2, 1, TileSize.MEDIUM),    # Bottom center medium
                TileSpec(2, 3, 1, 1, TileSize.SMALL),     # Bottom right small
                TileSpec(3, 0, 2, 1, TileSize.MEDIUM),    # Bottom left medium
                TileSpec(3, 2, 2, 1, TileSize.MEDIUM),    # Bottom right medium
            ]
        ))

        return patterns

    def setup_layout(self, container_widget: QWidget) -> None:
        """Setup the grid layout in the container widget."""
        # Clear existing layout
        if self.grid_layout:
            self.clear_layout()

        # Create new grid layout
        self.grid_layout = QGridLayout(container_widget)
        self.grid_layout.setSpacing(self.gutter_size)
        self.grid_layout.setContentsMargins(20, 20, 20, 20)  # Outer margins

        self.logger.debug("Grid layout setup complete")

    def activate_tile_mode(self, image_list: List[str]) -> None:
        """Activate tile layout mode with the given images."""
        if not self.grid_layout:
            self.logger.error("Grid layout not setup - call setup_layout first")
            return

        # Select random pattern
        pattern = random.choice(self.patterns)
        self.current_pattern = pattern

        # Create tiles for the pattern
        self._create_tiles_for_pattern(pattern, image_list)

        # Start shift animation timer
        self.shift_timer.start(self.shift_interval)

        self.logger.info(f"Activated tile mode with pattern: {pattern.name}")
        self.layout_changed.emit()

    def deactivate_tile_mode(self) -> None:
        """Deactivate tile layout mode."""
        self.shift_timer.stop()
        self.clear_layout()
        self.current_pattern = None

        self.logger.info("Deactivated tile mode")
        self.layout_changed.emit()

    def _create_tiles_for_pattern(self, pattern: BentoPattern, image_list: List[str]) -> None:
        """Create tile widgets for the given pattern."""
        # Clear existing tiles
        self.clear_layout()

        # Configure grid layout
        self.grid_layout.setColumnStretch(0, 1)
        self.grid_layout.setRowStretch(0, 1)

        # Shuffle images for variety
        available_images = image_list.copy()
        random.shuffle(available_images)

        # Create tiles
        for i, tile_spec in enumerate(pattern.tiles):
            if i < len(available_images):
                tile_spec.image_filename = available_images[i]

            # Create tile widget
            tile_widget = TileWidget(tile_spec)

            # Add to grid layout
            self.grid_layout.addWidget(
                tile_widget,
                tile_spec.row,
                tile_spec.col,
                tile_spec.height,  # row span
                tile_spec.width    # column span
            )

            self.tiles.append(tile_widget)

            # Load image asynchronously
            if tile_spec.image_filename:
                self._load_tile_image(tile_widget, tile_spec.image_filename)

        # Animate tiles in with staggered timing
        self._animate_tiles_in()

        self.logger.debug(f"Created {len(self.tiles)} tiles for pattern {pattern.name}")

    def _load_tile_image(self, tile_widget: TileWidget, filename: str) -> None:
        """Load image for a tile widget and extract color information."""
        # Check cache first
        if filename in self.image_cache:
            tile_widget.set_image(self.image_cache[filename])
            self._update_background_colors()
            return

        # Load from API (this should be done asynchronously in a real implementation)
        try:
            pixmap = self.api_client.get_image_pixmap(filename)
            if not pixmap.isNull():
                self.image_cache[filename] = pixmap
                tile_widget.set_image(pixmap)

                # Extract dominant color for background gradient
                self._extract_tile_color(pixmap, filename)
                self._update_background_colors()
        except Exception as e:
            self.logger.warning(f"Failed to load tile image {filename}: {e}")

    def _extract_tile_color(self, pixmap: QPixmap, filename: str) -> None:
        """Extract dominant color from tile image for background gradient."""
        try:
            # Get classification data which includes dominant color
            classification_data = self.api_client.classify_image(filename, include_metadata=True)
            if classification_data and 'metadata' in classification_data:
                dominant_color = classification_data['metadata'].get('dominant_color', '#666666')

                # Store color for gradient calculation
                if not hasattr(self, '_tile_colors'):
                    self._tile_colors = []

                self._tile_colors.append(dominant_color)

                # Keep only recent colors (max 8 for gradient)
                if len(self._tile_colors) > 8:
                    self._tile_colors = self._tile_colors[-8:]

        except Exception as e:
            self.logger.debug(f"Could not extract color from {filename}: {e}")

    def _update_background_colors(self) -> None:
        """Update background gradient based on current tile colors."""
        if not hasattr(self, '_tile_colors') or not self._tile_colors:
            return

        # Use the most recent tile colors for gradient
        colors = self._tile_colors[-4:] if len(self._tile_colors) >= 4 else self._tile_colors

        if colors:
            self.update_gradient_background(colors)

    def _animate_tiles_in(self) -> None:
        """Animate all tiles fading in with sophisticated staggered timing."""
        # Sort tiles by visual importance (larger tiles first, then position)
        sorted_tiles = sorted(self.tiles, key=lambda t: (
            -t.tile_spec.width * t.tile_spec.height,  # Larger tiles first
            t.tile_spec.row,  # Then by row
            t.tile_spec.col   # Then by column
        ))

        for i, tile in enumerate(sorted_tiles):
            # Variable stagger timing based on tile importance
            if tile.tile_spec.size_type in [TileSize.LARGE, TileSize.XLARGE]:
                delay = i * 100  # Faster for important tiles
                duration = 800   # Longer animation
            elif tile.tile_spec.size_type in [TileSize.MEDIUM, TileSize.WIDE, TileSize.TALL]:
                delay = i * 150  # Medium timing
                duration = 600   # Medium animation
            else:
                delay = i * 200  # Slower for small tiles
                duration = 500   # Shorter animation

            # Add some randomness for organic feel
            import random
            delay += random.randint(0, 100)

            QTimer.singleShot(delay, lambda t=tile, d=duration: t.fade_in(d))

    def shift_tiles(self) -> None:
        """Shift to new random pattern with smooth transitions."""
        if not self.current_pattern or not self.tiles:
            return

        # Get new pattern (different from current)
        available_patterns = [p for p in self.patterns if p.name != self.current_pattern.name]
        if not available_patterns:
            return

        new_pattern = random.choice(available_patterns)

        # Get current image list
        current_images = []
        for tile in self.tiles:
            if tile.tile_spec.image_filename:
                current_images.append(tile.tile_spec.image_filename)

        # Add some new random images for variety
        all_images = self.api_client.get_image_list()
        available_new = [img for img in all_images if img not in current_images]
        if available_new:
            # Replace 30% of images with new ones
            num_to_replace = max(1, len(current_images) // 3)
            random.shuffle(available_new)
            current_images[-num_to_replace:] = available_new[:num_to_replace]

        # Fade out current tiles
        for tile in self.tiles:
            tile.fade_out(400)

        # Switch to new pattern after fade out
        QTimer.singleShot(500, lambda: self._create_tiles_for_pattern(new_pattern, current_images))

        self.current_pattern = new_pattern
        self.logger.debug(f"Shifted to pattern: {new_pattern.name}")

    def shift_to_pattern(self, pattern_name: str) -> None:
        """Shift to a specific bento pattern by name."""
        if pattern_name not in self.bento_patterns:
            self.logger.warning(f"Unknown pattern: {pattern_name}")
            return

        if pattern_name == self.current_pattern:
            self.logger.debug(f"Already using pattern: {pattern_name}")
            return

        # Get current image list
        current_images = []
        for tile in self.tiles:
            if tile.tile_spec.image_filename:
                current_images.append(tile.tile_spec.image_filename)

        # Fade out current tiles
        for tile in self.tiles:
            tile.fade_out(400)

        # Switch to new pattern after fade out
        pattern = self.bento_patterns[pattern_name]
        QTimer.singleShot(500, lambda: self._complete_pattern_shift_to(pattern, current_images))

        self.current_pattern = pattern_name
        self.logger.debug(f"Shifting to pattern: {pattern_name}")

    def _complete_pattern_shift_to(self, pattern: BentoPattern, images: List[str]) -> None:
        """Complete the pattern shift to a specific pattern."""
        # Clear current layout
        self.clear_layout()

        # Build new pattern
        self._build_tile_grid(pattern)

        # Load images with the new pattern
        self.load_images(images)

    def clear_layout(self) -> None:
        """Clear all tiles from the layout."""
        if not self.grid_layout:
            return

        for tile in self.tiles:
            self.grid_layout.removeWidget(tile)
            tile.deleteLater()

        self.tiles.clear()

    def update_gradient_background(self, colors: List[str]) -> None:
        """Update background with gradient based on tile image colors."""
        if not colors or not self.parent_widget:
            return

        # Create gradient from averaged colors
        gradient = QLinearGradient(0, 0, 1, 1)
        gradient.setCoordinateMode(QLinearGradient.ObjectBoundingMode)

        # Add color stops
        for i, color in enumerate(colors[:4]):  # Max 4 colors
            position = i / max(1, len(colors) - 1) if len(colors) > 1 else 0
            gradient.setColorAt(position, QColor(color))

        # Apply gradient background
        palette = self.parent_widget.palette()
        palette.setBrush(QPalette.Window, QBrush(gradient))
        self.parent_widget.setPalette(palette)
        self.parent_widget.setAutoFillBackground(True)

        self.logger.debug(f"Applied gradient background with {len(colors)} colors")

    def is_active(self) -> bool:
        """Check if tile mode is currently active."""
        return self.current_pattern is not None

    def get_current_pattern_info(self) -> Dict[str, Any]:
        """Get information about the current pattern."""
        if not self.current_pattern:
            return {}

        return {
            "name": self.current_pattern.name,
            "grid_size": f"{self.current_pattern.grid_cols}x{self.current_pattern.grid_rows}",
            "tile_count": len(self.tiles),
            "total_tiles": self.current_pattern.total_tiles
        }
