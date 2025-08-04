"""
Advanced image classification and metadata analysis for Aetherwave.

This module provides sophisticated image analysis capabilities using
computer vision and color analysis libraries to extract meaningful
metadata from artwork.
"""

import cv2
import numpy as np
from typing import List, Dict, Tuple, Optional, Any
from pathlib import Path
import json
import logging
from PIL import Image, ImageStat
from colorthief import ColorThief
import math

logger = logging.getLogger(__name__)

class ColorAnalyzer:
    """
    Advanced color analysis for image classification.
    Uses multiple techniques to extract dominant colors and color harmony.
    """

    def __init__(self):
        self.color_thief = None

    def analyze_colors(self, image_path: str, max_colors: int = 5) -> Dict[str, Any]:
        """
        Extract dominant colors and color metadata from an image.

        Args:
            image_path: Path to the image file
            max_colors: Maximum number of dominant colors to extract

        Returns:
            Dictionary containing color analysis results
        """
        try:
            # Preprocess image for faster analysis if it's very large
            processed_image_path = self._preprocess_image_for_analysis(image_path)

            # Use ColorThief for dominant color extraction
            color_thief = ColorThief(processed_image_path)

            # Get dominant color (quality=10 for much faster processing)
            dominant_color = color_thief.get_color(quality=10)

            # Get color palette (quality=10 for much faster processing)
            palette = color_thief.get_palette(color_count=max_colors, quality=10)

            # Clean up temporary file if we created one
            if processed_image_path != image_path:
                Path(processed_image_path).unlink(missing_ok=True)

            # Convert RGB tuples to hex strings
            dominant_hex = self._rgb_to_hex(dominant_color)
            palette_hex = [self._rgb_to_hex(color) for color in palette]

            # Analyze color temperature
            temperature = self._analyze_color_temperature(dominant_color)

            # Analyze color harmony
            harmony = self._analyze_color_harmony(palette)

            # Calculate color diversity
            diversity = self._calculate_color_diversity(palette)

            return {
                "dominant_color": dominant_hex,
                "palette": palette_hex,
                "temperature": temperature,
                "harmony_type": harmony,
                "color_diversity": diversity,
                "brightness": self._calculate_brightness(dominant_color),
                "saturation": self._calculate_saturation(dominant_color)
            }

        except Exception as e:
            logger.error(f"Color analysis failed for {image_path}: {str(e)}")
            return self._get_fallback_colors()

    def _rgb_to_hex(self, rgb: Tuple[int, int, int]) -> str:
        """Convert RGB tuple to hex string."""
        return f"#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}"

    def _analyze_color_temperature(self, rgb: Tuple[int, int, int]) -> str:
        """Analyze color temperature (warm/cool/neutral)."""
        r, _, b = rgb

        # Simple heuristic based on red vs blue dominance
        if r > b + 30:
            return "warm"
        elif b > r + 30:
            return "cool"
        else:
            return "neutral"

    def _analyze_color_harmony(self, palette: List[Tuple[int, int, int]]) -> str:
        """Analyze the color harmony type in the palette."""
        if len(palette) < 2:
            return "monochromatic"

        # Convert to HSV for better harmony analysis
        hsv_colors = [self._rgb_to_hsv(color) for color in palette]
        hues = [hsv[0] for hsv in hsv_colors]

        # Calculate hue differences
        hue_diffs = []
        for i in range(len(hues)):
            for j in range(i + 1, len(hues)):
                diff = abs(hues[i] - hues[j])
                diff = min(diff, 360 - diff)  # Handle wraparound
                hue_diffs.append(diff)

        if not hue_diffs:
            return "monochromatic"

        avg_diff = sum(hue_diffs) / len(hue_diffs)

        if avg_diff < 30:
            return "monochromatic"
        elif avg_diff < 60:
            return "analogous"
        elif avg_diff < 120:
            return "triadic"
        else:
            return "complementary"

    def _rgb_to_hsv(self, rgb: Tuple[int, int, int]) -> Tuple[float, float, float]:
        """Convert RGB to HSV."""
        r, g, b = [x / 255.0 for x in rgb]

        max_val = max(r, g, b)
        min_val = min(r, g, b)
        diff = max_val - min_val

        # Hue calculation
        if diff == 0:
            h = 0
        elif max_val == r:
            h = (60 * ((g - b) / diff) + 360) % 360
        elif max_val == g:
            h = (60 * ((b - r) / diff) + 120) % 360
        else:
            h = (60 * ((r - g) / diff) + 240) % 360

        # Saturation calculation
        s = 0 if max_val == 0 else (diff / max_val)

        # Value calculation
        v = max_val

        return (h, s, v)

    def _calculate_color_diversity(self, palette: List[Tuple[int, int, int]]) -> float:
        """Calculate how diverse the color palette is (0-1)."""
        if len(palette) < 2:
            return 0.0

        # Calculate average distance between all color pairs
        total_distance = 0
        pair_count = 0

        for i in range(len(palette)):
            for j in range(i + 1, len(palette)):
                distance = self._color_distance(palette[i], palette[j])
                total_distance += distance
                pair_count += 1

        if pair_count == 0:
            return 0.0

        avg_distance = total_distance / pair_count
        # Normalize to 0-1 range (max possible distance is ~441 for RGB)
        return min(avg_distance / 441.0, 1.0)

    def _color_distance(self, color1: Tuple[int, int, int], color2: Tuple[int, int, int]) -> float:
        """Calculate Euclidean distance between two RGB colors."""
        return math.sqrt(sum((a - b) ** 2 for a, b in zip(color1, color2)))

    def _calculate_brightness(self, rgb: Tuple[int, int, int]) -> float:
        """Calculate perceptual brightness (0-1)."""
        r, g, b = rgb
        # Use perceptual brightness formula
        brightness = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0
        return round(brightness, 3)

    def _calculate_saturation(self, rgb: Tuple[int, int, int]) -> float:
        """Calculate saturation (0-1)."""
        r, g, b = [x / 255.0 for x in rgb]
        max_val = max(r, g, b)
        min_val = min(r, g, b)

        if max_val == 0:
            return 0.0

        saturation = (max_val - min_val) / max_val
        return round(saturation, 3)

    def _preprocess_image_for_analysis(self, image_path: str) -> str:
        """
        Preprocess large images for faster analysis by resizing if needed.
        Returns path to processed image (original or temporary resized version).
        """
        import os
        import tempfile
        from PIL import Image

        # Check file size (in MB)
        file_size_mb = os.path.getsize(image_path) / (1024 * 1024)

        # If file is smaller than 1MB, use original
        if file_size_mb < 1.0:
            return image_path

        try:
            # Open image and check dimensions
            with Image.open(image_path) as img:
                width, height = img.size

                # If image is already small enough, use original
                if width <= 800 and height <= 600:
                    return image_path

                # Calculate new dimensions (maintain aspect ratio)
                aspect_ratio = width / height
                if aspect_ratio > 1:  # Landscape
                    new_width = 800
                    new_height = int(800 / aspect_ratio)
                else:  # Portrait
                    new_height = 600
                    new_width = int(600 * aspect_ratio)

                # Resize image
                resized_img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)

                # Save to temporary file
                temp_fd, temp_path = tempfile.mkstemp(suffix='.png')
                os.close(temp_fd)  # Close file descriptor, we just need the path

                resized_img.save(temp_path, 'PNG', optimize=True)
                return temp_path

        except Exception as e:
            print(f"Warning: Could not preprocess image {image_path}: {e}")
            return image_path  # Fallback to original

    def _get_fallback_colors(self) -> Dict[str, Any]:
        """Return fallback color data when analysis fails."""
        return {
            "dominant_color": "#1a1a2e",
            "palette": ["#1a1a2e", "#16213e", "#0f3460"],
            "temperature": "cool",
            "harmony_type": "monochromatic",
            "color_diversity": 0.3,
            "brightness": 0.2,
            "saturation": 0.5
        }

class ImageComplexityAnalyzer:
    """
    Analyze image complexity using computer vision techniques.
    """

    def analyze_complexity(self, image_path: str) -> Dict[str, float]:
        """
        Analyze various aspects of image complexity.

        Args:
            image_path: Path to the image file

        Returns:
            Dictionary containing complexity metrics
        """
        try:
            # Load image with OpenCV
            image = cv2.imread(image_path)
            if image is None:
                logger.error(f"Could not load image: {image_path}")
                return self._get_fallback_complexity()

            # Convert to grayscale for some analyses
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

            # Calculate various complexity metrics
            edge_density = self._calculate_edge_density(gray)
            texture_complexity = self._calculate_texture_complexity(gray)
            color_complexity = self._calculate_color_complexity(image)
            contrast = self._calculate_contrast(gray)

            # Overall complexity score (weighted average)
            overall_complexity = (
                edge_density * 0.3 +
                texture_complexity * 0.3 +
                color_complexity * 0.2 +
                contrast * 0.2
            )

            return {
                "edge_density": round(edge_density, 3),
                "texture_complexity": round(texture_complexity, 3),
                "color_complexity": round(color_complexity, 3),
                "contrast": round(contrast, 3),
                "overall_complexity": round(overall_complexity, 3)
            }

        except Exception as e:
            logger.error(f"Complexity analysis failed for {image_path}: {str(e)}")
            return self._get_fallback_complexity()

    def _calculate_edge_density(self, gray_image: np.ndarray) -> float:
        """Calculate edge density using Canny edge detection."""
        edges = cv2.Canny(gray_image, 50, 150)
        edge_pixels = np.count_nonzero(edges)
        total_pixels = gray_image.shape[0] * gray_image.shape[1]
        return edge_pixels / total_pixels

    def _calculate_texture_complexity(self, gray_image: np.ndarray) -> float:
        """Calculate texture complexity using local binary patterns."""
        try:
            # Simple texture measure using Laplacian variance
            laplacian = cv2.Laplacian(gray_image, cv2.CV_64F)
            variance = laplacian.var()
            # Normalize to 0-1 range
            return min(variance / 10000.0, 1.0)
        except (cv2.error, AttributeError):
            return 0.5

    def _calculate_color_complexity(self, image: np.ndarray) -> float:
        """Calculate color complexity based on unique colors."""
        # Resize for performance
        small_image = cv2.resize(image, (100, 100))

        # Count unique colors
        pixels = small_image.reshape(-1, 3)
        unique_colors = len(np.unique(pixels.view(np.dtype((np.void, pixels.dtype.itemsize * 3)))))

        # Normalize (max possible unique colors in 100x100 = 10000)
        return min(unique_colors / 10000.0, 1.0)

    def _calculate_contrast(self, gray_image: np.ndarray) -> float:
        """Calculate image contrast using standard deviation."""
        return min(gray_image.std() / 128.0, 1.0)

    def _get_fallback_complexity(self) -> Dict[str, float]:
        """Return fallback complexity data."""
        return {
            "edge_density": 0.3,
            "texture_complexity": 0.5,
            "color_complexity": 0.4,
            "contrast": 0.6,
            "overall_complexity": 0.45
        }

class MoodAnalyzer:
    """
    Analyze image mood based on color, brightness, and composition.
    """

    def analyze_mood(self, color_data: Dict[str, Any], complexity_data: Dict[str, float]) -> Dict[str, Any]:
        """
        Determine image mood based on color and complexity analysis.

        Args:
            color_data: Results from color analysis
            complexity_data: Results from complexity analysis

        Returns:
            Dictionary containing mood analysis
        """
        try:
            brightness = color_data.get("brightness", 0.5)
            saturation = color_data.get("saturation", 0.5)
            temperature = color_data.get("temperature", "neutral")
            complexity = complexity_data.get("overall_complexity", 0.5)

            # Determine primary mood
            primary_mood = self._determine_primary_mood(brightness, saturation, temperature, complexity)

            # Calculate mood confidence
            confidence = self._calculate_mood_confidence(brightness, saturation, complexity)

            # Determine secondary characteristics
            energy_level = self._calculate_energy_level(saturation, complexity)
            emotional_tone = self._determine_emotional_tone(brightness, temperature)

            return {
                "primary_mood": primary_mood,
                "emotional_tone": emotional_tone,
                "energy_level": energy_level,
                "confidence": round(confidence, 3),
                "mood_factors": {
                    "brightness_influence": round(brightness, 3),
                    "saturation_influence": round(saturation, 3),
                    "complexity_influence": round(complexity, 3),
                    "temperature_influence": temperature
                }
            }

        except Exception as e:
            logger.error(f"Mood analysis failed: {str(e)}")
            return self._get_fallback_mood()

    def _determine_primary_mood(self, brightness: float, saturation: float,
                               temperature: str, complexity: float) -> str:
        """Determine the primary mood category."""

        # High brightness, high saturation
        if brightness > 0.7 and saturation > 0.5:
            return "energetic" if temperature == "warm" else "serene"

        # Low brightness
        if brightness < 0.3:
            return "dramatic" if complexity > 0.6 else "mysterious"

        # High saturation
        if saturation > 0.7:
            return "vibrant" if temperature == "warm" else "cinematic"

        # High complexity
        if complexity > 0.7:
            return "dynamic"

        # Bright but low saturation
        if brightness > 0.5 and saturation < 0.3:
            return "peaceful"

        return "balanced"

    def _calculate_mood_confidence(self, brightness: float, saturation: float, complexity: float) -> float:
        """Calculate confidence in mood classification."""
        # Higher confidence for more extreme values
        brightness_conf = abs(brightness - 0.5) * 2
        saturation_conf = abs(saturation - 0.5) * 2
        complexity_conf = abs(complexity - 0.5) * 2

        avg_confidence = (brightness_conf + saturation_conf + complexity_conf) / 3
        return min(max(avg_confidence, 0.3), 0.95)  # Clamp between 0.3 and 0.95

    def _calculate_energy_level(self, saturation: float, complexity: float) -> str:
        """Calculate energy level based on saturation and complexity."""
        energy_score = (saturation + complexity) / 2

        if energy_score > 0.7:
            return "high"
        elif energy_score > 0.4:
            return "medium"
        else:
            return "low"

    def _determine_emotional_tone(self, brightness: float, temperature: str) -> str:
        """Determine emotional tone."""
        if brightness > 0.6:
            if temperature == "warm":
                return "uplifting"
            else:
                return "calm"
        elif brightness < 0.4:
            if temperature == "cool":
                return "contemplative"
            else:
                return "intense"
        else:
            return "neutral"

    def _get_fallback_mood(self) -> Dict[str, Any]:
        """Return fallback mood data."""
        return {
            "primary_mood": "balanced",
            "emotional_tone": "neutral",
            "energy_level": "medium",
            "confidence": 0.5,
            "mood_factors": {
                "brightness_influence": 0.5,
                "saturation_influence": 0.5,
                "complexity_influence": 0.5,
                "temperature_influence": "neutral"
            }
        }

class AdvancedImageClassifier:
    """
    Main classifier that combines all analysis components.
    """

    def __init__(self):
        self.color_analyzer = ColorAnalyzer()
        self.complexity_analyzer = ImageComplexityAnalyzer()
        self.mood_analyzer = MoodAnalyzer()

    def classify_image(self, image_path: str) -> Dict[str, Any]:
        """
        Perform comprehensive image classification.

        Args:
            image_path: Path to the image file

        Returns:
            Complete classification results
        """
        try:
            logger.info(f"Starting advanced classification for: {image_path}")

            # Verify image exists
            if not Path(image_path).exists():
                raise FileNotFoundError(f"Image not found: {image_path}")

            # Get image basic info
            with Image.open(image_path) as img:
                width, height = img.size
                format_type = img.format

            # Perform analyses
            color_data = self.color_analyzer.analyze_colors(image_path)
            complexity_data = self.complexity_analyzer.analyze_complexity(image_path)
            mood_data = self.mood_analyzer.analyze_mood(color_data, complexity_data)

            # Combine results
            classification_result = {
                "basic_info": {
                    "filename": Path(image_path).name,
                    "width": width,
                    "height": height,
                    "aspect_ratio": round(width / height, 3),
                    "format": format_type,
                    "megapixels": round((width * height) / 1_000_000, 2)
                },
                "color_analysis": color_data,
                "complexity_analysis": complexity_data,
                "mood_analysis": mood_data,
                "classification_metadata": {
                    "analyzer_version": "2.0.0",
                    "confidence_score": mood_data.get("confidence", 0.5),
                    "processing_timestamp": self._get_timestamp(),
                    "recommended_display_duration": self._calculate_display_duration(complexity_data),
                    "cinematic_score": self._calculate_cinematic_score(color_data, mood_data)
                }
            }

            logger.info(f"Classification complete for: {image_path}")
            return classification_result

        except Exception as e:
            logger.error(f"Classification failed for {image_path}: {str(e)}")
            return self._get_fallback_classification(image_path)

    def _calculate_display_duration(self, complexity_data: Dict[str, float]) -> float:
        """Calculate recommended display duration based on complexity."""
        base_duration = 8.0  # seconds
        complexity = complexity_data.get("overall_complexity", 0.5)

        # More complex images should be displayed longer
        duration = base_duration + (complexity * 4.0)
        return round(duration, 1)

    def _calculate_cinematic_score(self, color_data: Dict[str, Any], mood_data: Dict[str, Any]) -> float:
        """Calculate how cinematic the image is (0-1)."""
        factors = []

        # Color factors
        if color_data.get("temperature") in ["cool", "warm"]:
            factors.append(0.7)
        else:
            factors.append(0.5)

        # Brightness factors (not too bright, not too dark)
        brightness = color_data.get("brightness", 0.5)
        brightness_score = 1.0 - abs(brightness - 0.4)  # Sweet spot around 0.4
        factors.append(brightness_score)

        # Mood factors
        cinematic_moods = ["dramatic", "mysterious", "cinematic", "contemplative"]
        if mood_data.get("primary_mood") in cinematic_moods:
            factors.append(0.8)
        else:
            factors.append(0.4)

        return round(sum(factors) / len(factors), 3)

    def _get_timestamp(self) -> str:
        """Get current timestamp in ISO format."""
        from datetime import datetime
        return datetime.now().isoformat()

    def _get_fallback_classification(self, image_path: str) -> Dict[str, Any]:
        """Return fallback classification when analysis fails."""
        return {
            "basic_info": {
                "filename": Path(image_path).name,
                "width": 1920,
                "height": 1080,
                "aspect_ratio": 1.778,
                "format": "Unknown",
                "megapixels": 2.07
            },
            "color_analysis": self.color_analyzer._get_fallback_colors(),
            "complexity_analysis": self.complexity_analyzer._get_fallback_complexity(),
            "mood_analysis": self.mood_analyzer._get_fallback_mood(),
            "classification_metadata": {
                "analyzer_version": "2.0.0",
                "confidence_score": 0.3,
                "processing_timestamp": self._get_timestamp(),
                "recommended_display_duration": 8.0,
                "cinematic_score": 0.5
            }
        }
