"""
Comprehensive test suite for Aetherwave advanced image classification.

Tests all components of the image analysis pipeline including color analysis,
complexity analysis, mood detection, and cinematic scoring.
"""

import pytest
import numpy as np
import cv2
from pathlib import Path
import tempfile
import json
from PIL import Image, ImageDraw
import sys
import os

# Add src/python to path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'src', 'api'))

from advanced_classifier import (
    ColorAnalyzer,
    ImageComplexityAnalyzer,
    MoodAnalyzer,
    AdvancedImageClassifier
)

class TestColorAnalyzer:
    """Test suite for color analysis functionality."""

    @pytest.fixture
    def color_analyzer(self):
        return ColorAnalyzer()

    @pytest.fixture
    def test_image_path(self):
        """Create a temporary test image."""
        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            # Create a simple test image with known colors
            img = Image.new('RGB', (100, 100), color='blue')
            draw = ImageDraw.Draw(img)
            draw.rectangle([25, 25, 75, 75], fill='red')
            img.save(f.name)
            yield f.name
        # Cleanup
        os.unlink(f.name)

    def test_rgb_to_hex_conversion(self, color_analyzer):
        """Test RGB to hex color conversion."""
        assert color_analyzer._rgb_to_hex((255, 0, 0)) == "#ff0000"
        assert color_analyzer._rgb_to_hex((0, 255, 0)) == "#00ff00"
        assert color_analyzer._rgb_to_hex((0, 0, 255)) == "#0000ff"
        assert color_analyzer._rgb_to_hex((128, 128, 128)) == "#808080"

    def test_color_temperature_analysis(self, color_analyzer):
        """Test color temperature classification."""
        # Red should be warm
        assert color_analyzer._analyze_color_temperature((255, 100, 100)) == "warm"
        # Blue should be cool
        assert color_analyzer._analyze_color_temperature((100, 100, 255)) == "cool"
        # Gray should be neutral
        assert color_analyzer._analyze_color_temperature((128, 128, 128)) == "neutral"

    def test_rgb_to_hsv_conversion(self, color_analyzer):
        """Test RGB to HSV color space conversion."""
        # Test pure red
        h, s, v = color_analyzer._rgb_to_hsv((255, 0, 0))
        assert abs(h - 0) < 1  # Red is at 0 degrees
        assert abs(s - 1) < 0.1  # High saturation
        assert abs(v - 1) < 0.1  # High value

        # Test gray (no saturation)
        h, s, v = color_analyzer._rgb_to_hsv((128, 128, 128))
        assert abs(s - 0) < 0.1  # No saturation

    def test_color_distance_calculation(self, color_analyzer):
        """Test color distance calculation."""
        # Same colors should have zero distance
        assert color_analyzer._color_distance((255, 0, 0), (255, 0, 0)) == 0

        # Black and white should have maximum distance
        distance = color_analyzer._color_distance((0, 0, 0), (255, 255, 255))
        expected = np.sqrt(3 * 255**2)  # √(255² + 255² + 255²)
        assert abs(distance - expected) < 1

    def test_brightness_calculation(self, color_analyzer):
        """Test perceptual brightness calculation."""
        # White should be bright
        assert color_analyzer._calculate_brightness((255, 255, 255)) == 1.0
        # Black should be dark
        assert color_analyzer._calculate_brightness((0, 0, 0)) == 0.0
        # Gray should be medium
        brightness = color_analyzer._calculate_brightness((128, 128, 128))
        assert 0.4 < brightness < 0.6

    def test_saturation_calculation(self, color_analyzer):
        """Test saturation calculation."""
        # Pure red should be highly saturated
        assert color_analyzer._calculate_saturation((255, 0, 0)) == 1.0
        # Gray should have no saturation
        assert color_analyzer._calculate_saturation((128, 128, 128)) == 0.0

    def test_color_harmony_analysis(self, color_analyzer):
        """Test color harmony classification."""
        # Monochromatic colors (similar hues)
        mono_palette = [(255, 200, 200), (255, 150, 150), (255, 100, 100)]
        harmony = color_analyzer._analyze_color_harmony(mono_palette)
        assert harmony in ["monochromatic", "analogous"]

        # Single color should be monochromatic
        single_palette = [(255, 0, 0)]
        assert color_analyzer._analyze_color_harmony(single_palette) == "monochromatic"

    def test_fallback_colors(self, color_analyzer):
        """Test fallback color data structure."""
        fallback = color_analyzer._get_fallback_colors()

        assert "dominant_color" in fallback
        assert "palette" in fallback
        assert "temperature" in fallback
        assert "harmony_type" in fallback
        assert "color_diversity" in fallback
        assert "brightness" in fallback
        assert "saturation" in fallback

        # Check that values are reasonable
        assert 0 <= fallback["brightness"] <= 1
        assert 0 <= fallback["saturation"] <= 1
        assert 0 <= fallback["color_diversity"] <= 1

class TestImageComplexityAnalyzer:
    """Test suite for image complexity analysis."""

    @pytest.fixture
    def complexity_analyzer(self):
        return ImageComplexityAnalyzer()

    @pytest.fixture
    def simple_test_image(self):
        """Create a simple test image with low complexity."""
        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            # Create solid color image (low complexity)
            img = Image.new('RGB', (100, 100), color='blue')
            img.save(f.name)
            yield f.name
        os.unlink(f.name)

    @pytest.fixture
    def complex_test_image(self):
        """Create a complex test image with high complexity."""
        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            # Create noisy image (high complexity)
            img = Image.new('RGB', (100, 100))
            pixels = []
            for i in range(10000):  # 100x100 pixels
                # Random colors for complexity
                pixels.append((
                    np.random.randint(0, 256),
                    np.random.randint(0, 256),
                    np.random.randint(0, 256)
                ))
            img.putdata(pixels)
            img.save(f.name)
            yield f.name
        os.unlink(f.name)

    def test_edge_density_calculation(self, complexity_analyzer):
        """Test edge density calculation."""
        # Create test image with known edge characteristics
        image = np.zeros((100, 100), dtype=np.uint8)
        # Add a vertical line (should create edges)
        image[:, 50] = 255

        edge_density = complexity_analyzer._calculate_edge_density(image)
        assert 0 <= edge_density <= 1
        # Should have some edges due to the line
        assert edge_density > 0

    def test_texture_complexity_calculation(self, complexity_analyzer):
        """Test texture complexity calculation."""
        # Smooth image (low texture)
        smooth_image = np.ones((100, 100), dtype=np.uint8) * 128
        texture_smooth = complexity_analyzer._calculate_texture_complexity(smooth_image)

        # Noisy image (high texture)
        noisy_image = np.random.randint(0, 256, (100, 100)).astype(np.uint8)
        texture_noisy = complexity_analyzer._calculate_texture_complexity(noisy_image)

        # Noisy image should have higher texture complexity
        assert texture_noisy >= texture_smooth
        assert 0 <= texture_smooth <= 1
        assert 0 <= texture_noisy <= 1

    def test_color_complexity_calculation(self, complexity_analyzer):
        """Test color complexity calculation."""
        # Single color image (low complexity)
        single_color = np.ones((100, 100, 3), dtype=np.uint8) * 128
        complexity_low = complexity_analyzer._calculate_color_complexity(single_color)

        # Random color image (high complexity)
        random_colors = np.random.randint(0, 256, (100, 100, 3)).astype(np.uint8)
        complexity_high = complexity_analyzer._calculate_color_complexity(random_colors)

        # Random colors should have higher complexity
        assert complexity_high >= complexity_low
        assert 0 <= complexity_low <= 1
        assert 0 <= complexity_high <= 1

    def test_contrast_calculation(self, complexity_analyzer):
        """Test contrast calculation."""
        # Low contrast image
        low_contrast = np.ones((100, 100), dtype=np.uint8) * 128
        contrast_low = complexity_analyzer._calculate_contrast(low_contrast)

        # High contrast image (black and white)
        high_contrast = np.zeros((100, 100), dtype=np.uint8)
        high_contrast[:50, :] = 255  # Half white, half black
        contrast_high = complexity_analyzer._calculate_contrast(high_contrast)

        # High contrast image should have higher contrast value
        assert contrast_high > contrast_low
        assert 0 <= contrast_low <= 1
        assert 0 <= contrast_high <= 1

    def test_complexity_analysis_structure(self, complexity_analyzer, simple_test_image):
        """Test that complexity analysis returns proper structure."""
        result = complexity_analyzer.analyze_complexity(simple_test_image)

        required_keys = [
            "edge_density", "texture_complexity",
            "color_complexity", "contrast", "overall_complexity"
        ]

        for key in required_keys:
            assert key in result
            assert 0 <= result[key] <= 1

    def test_fallback_complexity(self, complexity_analyzer):
        """Test fallback complexity data."""
        fallback = complexity_analyzer._get_fallback_complexity()

        required_keys = [
            "edge_density", "texture_complexity",
            "color_complexity", "contrast", "overall_complexity"
        ]

        for key in required_keys:
            assert key in fallback
            assert 0 <= fallback[key] <= 1

class TestMoodAnalyzer:
    """Test suite for mood analysis functionality."""

    @pytest.fixture
    def mood_analyzer(self):
        return MoodAnalyzer()

    @pytest.fixture
    def sample_color_data(self):
        return {
            "brightness": 0.7,
            "saturation": 0.8,
            "temperature": "warm",
            "dominant_color": "#ff6b6b",
            "palette": ["#ff6b6b", "#4ecdc4", "#45b7d1"]
        }

    @pytest.fixture
    def sample_complexity_data(self):
        return {
            "overall_complexity": 0.6,
            "edge_density": 0.5,
            "texture_complexity": 0.7,
            "color_complexity": 0.4,
            "contrast": 0.8
        }

    def test_primary_mood_determination(self, mood_analyzer):
        """Test primary mood classification logic."""
        # High brightness + high saturation + warm = energetic
        mood = mood_analyzer._determine_primary_mood(0.8, 0.7, "warm", 0.5)
        assert mood == "energetic"

        # Low brightness + high complexity = dramatic
        mood = mood_analyzer._determine_primary_mood(0.2, 0.5, "cool", 0.8)
        assert mood == "dramatic"

        # Low brightness + low complexity = mysterious
        mood = mood_analyzer._determine_primary_mood(0.2, 0.5, "cool", 0.3)
        assert mood == "mysterious"

        # High saturation + warm = vibrant
        mood = mood_analyzer._determine_primary_mood(0.5, 0.8, "warm", 0.5)
        assert mood == "vibrant"

        # High complexity = dynamic
        mood = mood_analyzer._determine_primary_mood(0.5, 0.5, "neutral", 0.8)
        assert mood == "dynamic"

        # Bright + low saturation = peaceful
        mood = mood_analyzer._determine_primary_mood(0.7, 0.2, "neutral", 0.4)
        assert mood == "peaceful"

        # Default case = balanced
        mood = mood_analyzer._determine_primary_mood(0.5, 0.5, "neutral", 0.5)
        assert mood == "balanced"

    def test_mood_confidence_calculation(self, mood_analyzer):
        """Test mood confidence calculation."""
        # Extreme values should give high confidence
        confidence_high = mood_analyzer._calculate_mood_confidence(0.9, 0.9, 0.9)
        assert confidence_high > 0.7

        # Middle values should give lower confidence
        confidence_low = mood_analyzer._calculate_mood_confidence(0.5, 0.5, 0.5)
        assert confidence_low < confidence_high

        # All confidences should be in valid range
        assert 0.3 <= confidence_high <= 0.95
        assert 0.3 <= confidence_low <= 0.95

    def test_energy_level_calculation(self, mood_analyzer):
        """Test energy level classification."""
        # High saturation + high complexity = high energy
        energy = mood_analyzer._calculate_energy_level(0.8, 0.8)
        assert energy == "high"

        # Medium values = medium energy
        energy = mood_analyzer._calculate_energy_level(0.5, 0.5)
        assert energy == "medium"

        # Low values = low energy
        energy = mood_analyzer._calculate_energy_level(0.2, 0.2)
        assert energy == "low"

    def test_emotional_tone_determination(self, mood_analyzer):
        """Test emotional tone classification."""
        # Bright + warm = uplifting
        tone = mood_analyzer._determine_emotional_tone(0.7, "warm")
        assert tone == "uplifting"

        # Bright + cool = calm
        tone = mood_analyzer._determine_emotional_tone(0.7, "cool")
        assert tone == "calm"

        # Dark + cool = contemplative
        tone = mood_analyzer._determine_emotional_tone(0.3, "cool")
        assert tone == "contemplative"

        # Dark + warm = intense
        tone = mood_analyzer._determine_emotional_tone(0.3, "warm")
        assert tone == "intense"

        # Medium = neutral
        tone = mood_analyzer._determine_emotional_tone(0.5, "neutral")
        assert tone == "neutral"

    def test_mood_analysis_integration(self, mood_analyzer, sample_color_data, sample_complexity_data):
        """Test complete mood analysis."""
        result = mood_analyzer.analyze_mood(sample_color_data, sample_complexity_data)

        required_keys = [
            "primary_mood", "emotional_tone", "energy_level",
            "confidence", "mood_factors"
        ]

        for key in required_keys:
            assert key in result

        # Check mood factors structure
        mood_factors = result["mood_factors"]
        factor_keys = [
            "brightness_influence", "saturation_influence",
            "complexity_influence", "temperature_influence"
        ]

        for key in factor_keys:
            assert key in mood_factors

        # Check value ranges
        assert 0.3 <= result["confidence"] <= 0.95
        assert result["energy_level"] in ["low", "medium", "high"]
        assert result["primary_mood"] in [
            "energetic", "serene", "dramatic", "mysterious",
            "vibrant", "cinematic", "dynamic", "peaceful", "balanced"
        ]

class TestAdvancedImageClassifier:
    """Test suite for the complete advanced image classifier."""

    @pytest.fixture
    def classifier(self):
        return AdvancedImageClassifier()

    @pytest.fixture
    def test_image_file(self):
        """Create a test image file."""
        with tempfile.NamedTemporaryFile(suffix='.jpg', delete=False) as f:
            # Create a test image
            img = Image.new('RGB', (800, 600), color='navy')
            draw = ImageDraw.Draw(img)
            # Add some visual complexity
            draw.ellipse([100, 100, 300, 300], fill='gold')
            draw.rectangle([400, 200, 600, 400], fill='crimson')
            img.save(f.name, 'JPEG')
            yield f.name
        os.unlink(f.name)

    def test_cinematic_score_calculation(self, classifier):
        """Test cinematic score calculation."""
        color_data = {
            "temperature": "cool",
            "brightness": 0.4  # Good cinematic brightness
        }
        mood_data = {
            "primary_mood": "cinematic"
        }

        score = classifier._calculate_cinematic_score(color_data, mood_data)
        assert 0 <= score <= 1

        # Cinematic mood should boost score
        assert score > 0.5

    def test_display_duration_calculation(self, classifier):
        """Test display duration calculation."""
        # Low complexity should have shorter duration
        low_complexity = {"overall_complexity": 0.2}
        duration_low = classifier._calculate_display_duration(low_complexity)

        # High complexity should have longer duration
        high_complexity = {"overall_complexity": 0.9}
        duration_high = classifier._calculate_display_duration(high_complexity)

        assert duration_high > duration_low
        assert duration_low >= 8.0  # Base duration
        assert duration_high <= 12.0  # Max reasonable duration

    def test_complete_classification(self, classifier, test_image_file):
        """Test complete image classification pipeline."""
        result = classifier.classify_image(test_image_file)

        # Check top-level structure
        main_sections = [
            "basic_info", "color_analysis", "complexity_analysis",
            "mood_analysis", "classification_metadata"
        ]

        for section in main_sections:
            assert section in result

        # Check basic info
        basic_info = result["basic_info"]
        assert basic_info["width"] == 800
        assert basic_info["height"] == 600
        assert abs(basic_info["aspect_ratio"] - (800/600)) < 0.01

        # Check metadata
        metadata = result["classification_metadata"]
        required_metadata = [
            "analyzer_version", "confidence_score", "processing_timestamp",
            "recommended_display_duration", "cinematic_score"
        ]

        for key in required_metadata:
            assert key in metadata

        # Check value ranges
        assert 0 <= metadata["confidence_score"] <= 1
        assert 0 <= metadata["cinematic_score"] <= 1
        assert metadata["recommended_display_duration"] >= 8.0

    def test_classification_error_handling(self, classifier):
        """Test error handling for invalid image paths."""
        # Non-existent file should return fallback
        result = classifier.classify_image("/nonexistent/path.jpg")

        # Should still return valid structure
        assert "basic_info" in result
        assert "classification_metadata" in result

        # Should have low confidence
        assert result["classification_metadata"]["confidence_score"] <= 0.5

    def test_timestamp_format(self, classifier):
        """Test timestamp format in metadata."""
        timestamp = classifier._get_timestamp()

        # Should be in ISO format
        from datetime import datetime
        try:
            datetime.fromisoformat(timestamp)
            timestamp_valid = True
        except ValueError:
            timestamp_valid = False

        assert timestamp_valid

class TestIntegration:
    """Integration tests for the complete system."""

    def test_color_to_mood_pipeline(self):
        """Test the pipeline from color analysis to mood determination."""
        color_analyzer = ColorAnalyzer()
        mood_analyzer = MoodAnalyzer()

        # Create test data that should produce predictable results
        warm_bright_colors = {
            "brightness": 0.8,
            "saturation": 0.7,
            "temperature": "warm",
            "dominant_color": "#ff6b6b"
        }

        medium_complexity = {
            "overall_complexity": 0.5,
            "edge_density": 0.4,
            "texture_complexity": 0.6
        }

        mood_result = mood_analyzer.analyze_mood(warm_bright_colors, medium_complexity)

        # Warm, bright colors should tend toward energetic moods
        assert mood_result["primary_mood"] in ["energetic", "vibrant", "uplifting"]
        assert mood_result["energy_level"] in ["medium", "high"]

    def test_complexity_to_mood_pipeline(self):
        """Test how complexity affects mood classification."""
        mood_analyzer = MoodAnalyzer()

        neutral_colors = {
            "brightness": 0.5,
            "saturation": 0.5,
            "temperature": "neutral"
        }

        # High complexity should lead to dynamic mood
        high_complexity = {"overall_complexity": 0.9}
        mood_high = mood_analyzer.analyze_mood(neutral_colors, high_complexity)

        # Low complexity should lead to calmer moods
        low_complexity = {"overall_complexity": 0.2}
        mood_low = mood_analyzer.analyze_mood(neutral_colors, low_complexity)

        # High complexity should have higher energy
        assert mood_high["energy_level"] != "low"
        # Complex images are more likely to be dynamic
        dynamic_moods = ["dynamic", "dramatic", "energetic"]
        assert mood_high["primary_mood"] in dynamic_moods or mood_low["primary_mood"] != mood_high["primary_mood"]

if __name__ == "__main__":
    # Run tests with pytest
    pytest.main([__file__, "-v", "--tb=short"])
