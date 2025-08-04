"""
Tests for content-driven theme analysis functionality.

This module tests the CollectionAnalyzer and theme detection capabilities
for automatic aesthetic adaptation based on image content.
"""

import pytest
import json
import tempfile
from pathlib import Path
from unittest.mock import Mock, patch
import sys
import os

# Add src path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "src", "api"))

from content_theme_analyzer import (
    CollectionAnalyzer, ThemeProfile, ContentThemeCache
)

class TestThemeProfile:
    """Test suite for ThemeProfile dataclass."""

    def test_theme_profile_creation(self):
        """Test ThemeProfile instantiation and data access."""
        theme = ThemeProfile(
            theme_name="cyberfemme",
            confidence=0.85,
            primary_colors=["#1a1a2e", "#16213e"],
            accent_colors=["#ff00ff", "#00ffff"],
            temperature_bias="cool",
            mood_profile={"vibrant": 0.6, "cinematic": 0.4},
            energy_level="high",
            complexity_preference=0.7,
            harmony_types=["triadic", "complementary"]
        )

        assert theme.theme_name == "cyberfemme"
        assert theme.confidence == pytest.approx(0.85, rel=1e-2)
        assert len(theme.primary_colors) == 2
        assert len(theme.accent_colors) == 2
        assert theme.temperature_bias == "cool"
        assert theme.energy_level == "high"

    def test_theme_profile_to_dict(self):
        """Test ThemeProfile serialization to dictionary."""
        theme = ThemeProfile(
            theme_name="organic",
            confidence=0.75,
            primary_colors=["#2e4a3d"],
            accent_colors=["#8fbc8f"],
            temperature_bias="warm",
            mood_profile={"peaceful": 0.8},
            energy_level="medium",
            complexity_preference=0.4,
            harmony_types=["analogous"]
        )

        theme_dict = theme.to_dict()

        assert isinstance(theme_dict, dict)
        assert theme_dict["theme_name"] == "organic"
        assert theme_dict["confidence"] == pytest.approx(0.75, rel=1e-2)
        assert "primary_colors" in theme_dict
        assert "mood_profile" in theme_dict


class TestCollectionAnalyzer:
    """Test suite for CollectionAnalyzer functionality."""

    @pytest.fixture
    def collection_analyzer(self):
        """Create a CollectionAnalyzer instance."""
        return CollectionAnalyzer()

    @pytest.fixture
    def sample_metadata_files(self):
        """Create sample metadata files for testing."""
        return [
            {
                "color_analysis": {
                    "dominant_color": "#ff00ff",
                    "palette": ["#ff00ff", "#9932cc", "#4b0082"],
                    "temperature": "cool",
                    "harmony_type": "triadic",
                    "brightness": 0.6,
                    "saturation": 0.8
                },
                "mood_analysis": {
                    "primary_mood": "vibrant",
                    "emotional_tone": "uplifting",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.7
                }
            },
            {
                "color_analysis": {
                    "dominant_color": "#9370db",
                    "palette": ["#9370db", "#da70d6", "#ff1493"],
                    "temperature": "cool",
                    "harmony_type": "analogous",
                    "brightness": 0.5,
                    "saturation": 0.9
                },
                "mood_analysis": {
                    "primary_mood": "cinematic",
                    "emotional_tone": "intense",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.8
                }
            },
            {
                "color_analysis": {
                    "dominant_color": "#00ffff",
                    "palette": ["#00ffff", "#1e90ff", "#0080ff"],
                    "temperature": "cool",
                    "harmony_type": "monochromatic",
                    "brightness": 0.7,
                    "saturation": 0.6
                },
                "mood_analysis": {
                    "primary_mood": "dramatic",
                    "emotional_tone": "contemplative",
                    "energy_level": "medium"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.6
                }
            }
        ]

    @pytest.fixture
    def temp_metadata_dir(self, sample_metadata_files):
        """Create temporary directory with sample metadata files."""
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)

            # Create sample metadata files
            for i, metadata in enumerate(sample_metadata_files):
                json_file = temp_path / f"image_{i+1}.json"
                with open(json_file, 'w') as f:
                    json.dump(metadata, f, indent=2)

            yield str(temp_path)

    def test_load_metadata_files(self, collection_analyzer, temp_metadata_dir):
        """Test loading metadata files from directory."""
        metadata_files = collection_analyzer._load_metadata_files(temp_metadata_dir)

        assert len(metadata_files) == 3
        assert all(isinstance(metadata, dict) for metadata in metadata_files)
        assert all("color_analysis" in metadata for metadata in metadata_files)

    def test_load_metadata_files_empty_dir(self, collection_analyzer):
        """Test loading from empty directory."""
        with tempfile.TemporaryDirectory() as temp_dir:
            metadata_files = collection_analyzer._load_metadata_files(temp_dir)
            assert metadata_files == []

    def test_load_metadata_files_nonexistent_dir(self, collection_analyzer):
        """Test loading from non-existent directory."""
        metadata_files = collection_analyzer._load_metadata_files("/nonexistent/path")
        assert metadata_files == []

    def test_aggregate_color_data(self, collection_analyzer, sample_metadata_files):
        """Test color data aggregation."""
        color_data = collection_analyzer._aggregate_color_data(sample_metadata_files)

        assert "dominant_colors" in color_data
        assert "temperature_bias" in color_data
        assert "harmony_types" in color_data
        assert "avg_brightness" in color_data
        assert "avg_saturation" in color_data

        # Check that cyberfemme colors are detected
        dominant_colors_str = " ".join(color_data["dominant_colors"]).lower()
        assert any(color in dominant_colors_str for color in ["ff00ff", "9370db", "00ffff"])

        # Check temperature bias (should be cool)
        assert color_data["temperature_bias"] == "cool"

        # Check brightness and saturation averages
        assert 0.5 <= color_data["avg_brightness"] <= 0.7
        assert 0.7 <= color_data["avg_saturation"] <= 0.9

    def test_aggregate_mood_data(self, collection_analyzer, sample_metadata_files):
        """Test mood data aggregation."""
        mood_data = collection_analyzer._aggregate_mood_data(sample_metadata_files)

        assert "mood_distribution" in mood_data
        assert "dominant_tone" in mood_data
        assert "dominant_energy" in mood_data

        # Check mood distribution
        mood_dist = mood_data["mood_distribution"]
        assert isinstance(mood_dist, dict)
        assert sum(mood_dist.values()) == pytest.approx(1.0, rel=1e-2)

        # Check that high-energy moods are present
        assert "vibrant" in mood_dist or "cinematic" in mood_dist

        # Check dominant energy level
        assert mood_data["dominant_energy"] == "high"

    def test_aggregate_complexity_data(self, collection_analyzer, sample_metadata_files):
        """Test complexity data aggregation."""
        complexity_data = collection_analyzer._aggregate_complexity_data(sample_metadata_files)

        assert "avg_complexity" in complexity_data
        assert "complexity_variance" in complexity_data

        # Check average complexity
        assert 0.6 <= complexity_data["avg_complexity"] <= 0.8
        assert complexity_data["complexity_variance"] >= 0.0

    def test_has_cyberfemme_colors(self, collection_analyzer):
        """Test cyberfemme color detection."""
        # Should detect cyberfemme colors
        cyberfemme_colors = ["#ff00ff", "#9932cc", "#da70d6", "#00ffff"]
        assert collection_analyzer._has_cyberfemme_colors(cyberfemme_colors)

        # Should not detect from earth tones
        earth_colors = ["#8b4513", "#cd853f", "#daa520"]
        assert not collection_analyzer._has_cyberfemme_colors(earth_colors)

    def test_has_earth_tones(self, collection_analyzer):
        """Test earth tone color detection."""
        # Should detect earth tones
        earth_colors = ["#8b4513", "#cd853f", "#daa520", "#a0522d"]
        assert collection_analyzer._has_earth_tones(earth_colors)

        # Should not detect from cyberfemme colors
        cyberfemme_colors = ["#ff00ff", "#9932cc", "#00ffff"]
        assert not collection_analyzer._has_earth_tones(cyberfemme_colors)

    def test_has_tech_colors(self, collection_analyzer):
        """Test tech color detection."""
        # Should detect tech colors
        tech_colors = ["#000000", "#ffffff", "#808080", "#0080ff"]
        assert collection_analyzer._has_tech_colors(tech_colors)

        # Should not detect from warm colors
        warm_colors = ["#ff6347", "#daa520", "#cd853f"]
        assert not collection_analyzer._has_tech_colors(warm_colors)

    def test_has_vintage_colors(self, collection_analyzer):
        """Test vintage color detection."""
        # Should detect vintage colors
        vintage_colors = ["#b8860b", "#cd853f", "#daa520", "#f5deb3"]
        assert collection_analyzer._has_vintage_colors(vintage_colors)

        # Should not detect from tech colors
        tech_colors = ["#000000", "#ffffff", "#0080ff"]
        assert not collection_analyzer._has_vintage_colors(tech_colors)

    def test_calculate_cyberfemme_score(self, collection_analyzer):
        """Test cyberfemme theme score calculation."""
        color_data = {
            "dominant_colors": ["#ff00ff", "#9932cc"],
            "avg_saturation": 0.8
        }
        mood_data = {
            "mood_distribution": {"vibrant": 0.5, "cinematic": 0.3},
            "dominant_energy": "high"
        }

        score = collection_analyzer._calculate_cyberfemme_score(color_data, mood_data)

        # Should be high score due to cyberfemme colors + high saturation + vibrant mood + high energy
        assert score >= 0.8

    def test_calculate_organic_score(self, collection_analyzer):
        """Test organic theme score calculation."""
        color_data = {
            "dominant_colors": ["#8b4513", "#cd853f"],
            "temperature_bias": "warm"
        }
        mood_data = {
            "mood_distribution": {"peaceful": 0.6, "serene": 0.4},
            "dominant_energy": "medium"
        }

        score = collection_analyzer._calculate_organic_score(color_data, mood_data)

        # Should be high score due to earth tones + warm temperature + peaceful mood + medium energy
        assert score >= 0.8

    def test_generate_theme_profile_cyberfemme(self, collection_analyzer):
        """Test theme profile generation for cyberfemme content."""
        color_data = {
            "dominant_colors": ["#ff00ff", "#9932cc", "#00ffff"],
            "temperature_bias": "cool",
            "harmony_types": ["triadic", "complementary"],
            "avg_brightness": 0.6,
            "avg_saturation": 0.8,
            "color_diversity": 0.7
        }
        mood_data = {
            "mood_distribution": {"vibrant": 0.4, "cinematic": 0.4, "dramatic": 0.2},
            "dominant_tone": "uplifting",
            "dominant_energy": "high"
        }
        complexity_data = {
            "avg_complexity": 0.7,
            "complexity_variance": 0.1
        }

        theme = collection_analyzer._generate_theme_profile(color_data, mood_data, complexity_data)

        assert theme.theme_name == "cyberfemme"
        assert theme.confidence >= 0.7
        assert theme.temperature_bias == "cool"
        assert theme.energy_level == "high"
        assert len(theme.primary_colors) >= 3
        assert len(theme.accent_colors) >= 1

    def test_analyze_collection_theme_integration(self, collection_analyzer, temp_metadata_dir):
        """Test complete collection theme analysis."""
        theme = collection_analyzer.analyze_collection_theme(temp_metadata_dir)

        assert isinstance(theme, ThemeProfile)
        assert theme.theme_name in ["cyberfemme", "organic", "tech", "vintage", "adaptive"]
        assert 0.3 <= theme.confidence <= 0.95
        assert len(theme.primary_colors) >= 1
        assert theme.energy_level in ["low", "medium", "high"]
        assert theme.temperature_bias in ["warm", "cool", "neutral"]

    def test_analyze_collection_theme_empty_directory(self, collection_analyzer):
        """Test collection analysis with empty directory."""
        with tempfile.TemporaryDirectory() as temp_dir:
            theme = collection_analyzer.analyze_collection_theme(temp_dir)

            # Should return fallback theme
            assert theme.theme_name == "adaptive"
            assert theme.confidence == pytest.approx(0.5, rel=1e-2)

    def test_get_fallback_theme(self, collection_analyzer):
        """Test fallback theme generation."""
        theme = collection_analyzer._get_fallback_theme()

        assert theme.theme_name == "adaptive"
        assert theme.confidence == pytest.approx(0.5, rel=1e-2)
        assert len(theme.primary_colors) >= 1
        assert len(theme.accent_colors) >= 1
        assert theme.energy_level == "medium"


class TestContentThemeCache:
    """Test suite for ContentThemeCache functionality."""

    @pytest.fixture
    def temp_cache_file(self):
        """Create temporary cache file."""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
            temp_path = f.name

        yield temp_path

        # Cleanup
        try:
            os.unlink(temp_path)
        except OSError:
            pass

    @pytest.fixture
    def sample_theme(self):
        """Create sample theme for caching tests."""
        return ThemeProfile(
            theme_name="cyberfemme",
            confidence=0.85,
            primary_colors=["#1a1a2e", "#16213e"],
            accent_colors=["#ff00ff", "#00ffff"],
            temperature_bias="cool",
            mood_profile={"vibrant": 0.6},
            energy_level="high",
            complexity_preference=0.7,
            harmony_types=["triadic"]
        )

    def test_cache_initialization(self, temp_cache_file):
        """Test cache initialization."""
        cache = ContentThemeCache(temp_cache_file)
        assert isinstance(cache.cache, dict)

    def test_cache_set_and_get(self, temp_cache_file, sample_theme):
        """Test caching and retrieving themes."""
        cache = ContentThemeCache(temp_cache_file)

        collection_hash = "test_collection_123"
        cache.set_theme(collection_hash, sample_theme)

        retrieved_theme = cache.get_theme(collection_hash)

        assert retrieved_theme is not None
        assert retrieved_theme.theme_name == sample_theme.theme_name
        assert retrieved_theme.confidence == pytest.approx(sample_theme.confidence, rel=1e-2)
        assert retrieved_theme.primary_colors == sample_theme.primary_colors

    def test_cache_get_nonexistent(self, temp_cache_file):
        """Test retrieving non-existent theme from cache."""
        cache = ContentThemeCache(temp_cache_file)

        retrieved_theme = cache.get_theme("nonexistent_hash")
        assert retrieved_theme is None

    def test_cache_persistence(self, temp_cache_file, sample_theme):
        """Test cache persistence across instances."""
        collection_hash = "persistent_test_456"

        # Set theme in first cache instance
        cache1 = ContentThemeCache(temp_cache_file)
        cache1.set_theme(collection_hash, sample_theme)

        # Retrieve theme in second cache instance
        cache2 = ContentThemeCache(temp_cache_file)
        retrieved_theme = cache2.get_theme(collection_hash)

        assert retrieved_theme is not None
        assert retrieved_theme.theme_name == sample_theme.theme_name
