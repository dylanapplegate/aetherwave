"""
Content-Driven Theme Analysis for Aetherwave

This module analyzes image collections to automatically detect aesthetic themes
and visual patterns, enabling adaptive UI generation based on content characteristics.
"""

from typing import Dict, List, Tuple, Any, Optional
from pathlib import Path
import json
import logging
import numpy as np
from collections import Counter, defaultdict
import statistics
from dataclasses import dataclass

logger = logging.getLogger(__name__)

@dataclass
class ThemeProfile:
    """
    Represents an aesthetic theme derived from content analysis.
    """
    theme_name: str
    confidence: float
    primary_colors: List[str]
    accent_colors: List[str]
    temperature_bias: str  # warm/cool/neutral
    mood_profile: Dict[str, float]
    energy_level: str  # low/medium/high
    complexity_preference: float  # 0-1
    harmony_types: List[str]

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization."""
        return {
            "theme_name": self.theme_name,
            "confidence": self.confidence,
            "primary_colors": self.primary_colors,
            "accent_colors": self.accent_colors,
            "temperature_bias": self.temperature_bias,
            "mood_profile": self.mood_profile,
            "energy_level": self.energy_level,
            "complexity_preference": self.complexity_preference,
            "harmony_types": self.harmony_types
        }

class CollectionAnalyzer:
    """
    Analyzes image collections to extract dominant aesthetic themes.
    """

    def __init__(self):
        """Initialize the collection analyzer."""
        self.metadata_cache: Dict[str, Dict] = {}

    def analyze_collection_theme(self, metadata_directory: str) -> ThemeProfile:
        """
        Analyze a collection of image metadata to determine the dominant aesthetic theme.

        Args:
            metadata_directory: Path to directory containing image metadata JSON files

        Returns:
            ThemeProfile representing the dominant aesthetic theme
        """
        try:
            logger.info(f"Analyzing collection theme from: {metadata_directory}")

            # Load all metadata files
            metadata_files = self._load_metadata_files(metadata_directory)

            if not metadata_files:
                logger.warning("No metadata files found, using fallback theme")
                return self._get_fallback_theme()

            logger.info(f"Found {len(metadata_files)} images to analyze")

            # Aggregate analysis data
            color_analysis = self._aggregate_color_data(metadata_files)
            mood_analysis = self._aggregate_mood_data(metadata_files)
            complexity_analysis = self._aggregate_complexity_data(metadata_files)

            # Generate theme profile
            theme_profile = self._generate_theme_profile(
                color_analysis, mood_analysis, complexity_analysis
            )

            logger.info(f"Generated theme: {theme_profile.theme_name} (confidence: {theme_profile.confidence:.2f})")
            return theme_profile

        except Exception as e:
            logger.error(f"Collection theme analysis failed: {str(e)}")
            return self._get_fallback_theme()

    def _load_metadata_files(self, metadata_directory: str) -> List[Dict[str, Any]]:
        """Load and parse all metadata JSON files."""
        metadata_files = []
        metadata_dir = Path(metadata_directory)

        if not metadata_dir.exists():
            logger.warning(f"Metadata directory not found: {metadata_directory}")
            return []

        for json_file in metadata_dir.glob("*.json"):
            try:
                with open(json_file, 'r') as f:
                    metadata = json.load(f)
                    metadata_files.append(metadata)
            except (json.JSONDecodeError, IOError) as e:
                logger.warning(f"Failed to load {json_file}: {str(e)}")
                continue

        return metadata_files

    def _aggregate_color_data(self, metadata_files: List[Dict]) -> Dict[str, Any]:
        """Aggregate color analysis across all images."""
        all_colors = []
        all_palettes = []
        temperatures = []
        harmonies = []
        brightness_values = []
        saturation_values = []

        for metadata in metadata_files:
            color_data = metadata.get("color_analysis", {})

            # Collect dominant colors
            dominant = color_data.get("dominant_color")
            if dominant:
                all_colors.append(dominant)

            # Collect palette colors
            palette = color_data.get("palette", [])
            all_palettes.extend(palette)

            # Collect other metrics
            temperatures.append(color_data.get("temperature", "neutral"))
            harmonies.append(color_data.get("harmony_type", "balanced"))
            brightness_values.append(color_data.get("brightness", 0.5))
            saturation_values.append(color_data.get("saturation", 0.5))

        # Find most common colors
        color_counter = Counter(all_colors + all_palettes)
        dominant_colors = [color for color, _ in color_counter.most_common(10)]

        # Determine temperature bias
        temp_counter = Counter(temperatures)
        dominant_temperature = temp_counter.most_common(1)[0][0] if temperatures else "neutral"

        # Determine harmony preferences
        harmony_counter = Counter(harmonies)
        common_harmonies = [harmony for harmony, _ in harmony_counter.most_common(3)]

        return {
            "dominant_colors": dominant_colors,
            "temperature_bias": dominant_temperature,
            "harmony_types": common_harmonies,
            "avg_brightness": statistics.mean(brightness_values) if brightness_values else 0.5,
            "avg_saturation": statistics.mean(saturation_values) if saturation_values else 0.5,
            "color_diversity": len(set(all_palettes)) / max(len(all_palettes), 1)
        }

    def _aggregate_mood_data(self, metadata_files: List[Dict]) -> Dict[str, Any]:
        """Aggregate mood analysis across all images."""
        moods = []
        tones = []
        energy_levels = []

        for metadata in metadata_files:
            mood_data = metadata.get("mood_analysis", {})

            moods.append(mood_data.get("primary_mood", "balanced"))
            tones.append(mood_data.get("emotional_tone", "neutral"))
            energy_levels.append(mood_data.get("energy_level", "medium"))

        # Create mood distribution
        mood_counter = Counter(moods)
        mood_distribution = {
            mood: count / len(moods) for mood, count in mood_counter.items()
        } if moods else {"balanced": 1.0}

        # Determine dominant characteristics
        tone_counter = Counter(tones)
        dominant_tone = tone_counter.most_common(1)[0][0] if tones else "neutral"

        energy_counter = Counter(energy_levels)
        dominant_energy = energy_counter.most_common(1)[0][0] if energy_levels else "medium"

        return {
            "mood_distribution": mood_distribution,
            "dominant_tone": dominant_tone,
            "dominant_energy": dominant_energy
        }

    def _aggregate_complexity_data(self, metadata_files: List[Dict]) -> Dict[str, Any]:
        """Aggregate complexity analysis across all images."""
        complexity_values = []

        for metadata in metadata_files:
            complexity_data = metadata.get("complexity_analysis", {})
            complexity_values.append(complexity_data.get("overall_complexity", 0.5))

        if not complexity_values:
            return {"avg_complexity": 0.5, "complexity_variance": 0.0}

        return {
            "avg_complexity": statistics.mean(complexity_values),
            "complexity_variance": statistics.variance(complexity_values) if len(complexity_values) > 1 else 0.0
        }

    def _generate_theme_profile(self, color_data: Dict, mood_data: Dict, complexity_data: Dict) -> ThemeProfile:
        """Generate a theme profile based on aggregated analysis."""

        # Detect theme based on content patterns
        theme_name, confidence = self._detect_theme_pattern(color_data, mood_data, complexity_data)

        # Generate color palettes
        primary_colors, accent_colors = self._generate_color_palettes(color_data, theme_name)

        return ThemeProfile(
            theme_name=theme_name,
            confidence=confidence,
            primary_colors=primary_colors,
            accent_colors=accent_colors,
            temperature_bias=color_data["temperature_bias"],
            mood_profile=mood_data["mood_distribution"],
            energy_level=mood_data["dominant_energy"],
            complexity_preference=complexity_data["avg_complexity"],
            harmony_types=color_data["harmony_types"]
        )

    def _detect_theme_pattern(self, color_data: Dict, mood_data: Dict, complexity_data: Dict) -> Tuple[str, float]:
        """
        Detect the most appropriate theme based on content patterns.

        Returns:
            Tuple of (theme_name, confidence_score)
        """
        # Calculate theme scores
        theme_scores = {
            "cyberfemme": self._calculate_cyberfemme_score(color_data, mood_data),
            "organic": self._calculate_organic_score(color_data, mood_data),
            "tech": self._calculate_tech_score(color_data, mood_data, complexity_data),
            "vintage": self._calculate_vintage_score(color_data, mood_data)
        }

        # Find best theme
        if not theme_scores or max(theme_scores.values()) < 0.3:
            return "adaptive", 0.5  # Fallback for mixed/unclear collections

        best_theme = max(theme_scores.keys(), key=lambda k: theme_scores[k])
        confidence = min(theme_scores[best_theme], 0.95)

        return best_theme, confidence

    def _calculate_cyberfemme_score(self, color_data: Dict, mood_data: Dict) -> float:
        """Calculate cyberfemme theme score based on content patterns."""
        score = 0.0
        dominant_colors = color_data["dominant_colors"]
        avg_saturation = color_data["avg_saturation"]
        mood_dist = mood_data["mood_distribution"]
        energy_level = mood_data["dominant_energy"]

        if self._has_cyberfemme_colors(dominant_colors):
            score += 0.4
        if avg_saturation > 0.6:
            score += 0.2
        if any(mood in ["vibrant", "cinematic", "dramatic", "energetic"] for mood in mood_dist.keys()):
            score += 0.3
        if energy_level == "high":
            score += 0.1

        return score

    def _calculate_organic_score(self, color_data: Dict, mood_data: Dict) -> float:
        """Calculate organic/natural theme score based on content patterns."""
        score = 0.0
        dominant_colors = color_data["dominant_colors"]
        temperature = color_data["temperature_bias"]
        mood_dist = mood_data["mood_distribution"]
        energy_level = mood_data["dominant_energy"]

        if self._has_earth_tones(dominant_colors):
            score += 0.4
        if temperature == "warm":
            score += 0.2
        if any(mood in ["peaceful", "serene", "balanced"] for mood in mood_dist.keys()):
            score += 0.3
        if energy_level in ["low", "medium"]:
            score += 0.1

        return score

    def _calculate_tech_score(self, color_data: Dict, mood_data: Dict, complexity_data: Dict) -> float:
        """Calculate tech/minimal theme score based on content patterns."""
        score = 0.0
        dominant_colors = color_data["dominant_colors"]
        temperature = color_data["temperature_bias"]
        avg_brightness = color_data["avg_brightness"]
        mood_dist = mood_data["mood_distribution"]
        complexity = complexity_data["avg_complexity"]

        if self._has_tech_colors(dominant_colors):
            score += 0.4
        if temperature == "cool":
            score += 0.2
        if any(mood in ["dynamic", "dramatic", "cinematic"] for mood in mood_dist.keys()):
            score += 0.2
        if complexity > 0.6:
            score += 0.1
        if avg_brightness < 0.4 or avg_brightness > 0.8:  # High contrast
            score += 0.1

        return score

    def _calculate_vintage_score(self, color_data: Dict, mood_data: Dict) -> float:
        """Calculate vintage/retro theme score based on content patterns."""
        score = 0.0
        dominant_colors = color_data["dominant_colors"]
        temperature = color_data["temperature_bias"]
        avg_saturation = color_data["avg_saturation"]
        avg_brightness = color_data["avg_brightness"]
        mood_dist = mood_data["mood_distribution"]

        if self._has_vintage_colors(dominant_colors):
            score += 0.4
        if temperature == "warm" and avg_saturation < 0.6:
            score += 0.3
        if any(mood in ["contemplative", "peaceful", "balanced"] for mood in mood_dist.keys()):
            score += 0.2
        if 0.3 < avg_brightness < 0.7:  # Mid-tone preference
            score += 0.1

        return score

    def _has_cyberfemme_colors(self, colors: List[str]) -> bool:
        """Check if color palette contains cyberfemme colors (purples, pinks, blues, magentas)."""
        cyberfemme_indicators = [
            # Magentas and hot pinks
            "ff00ff", "ff1493", "ff69b4", "da70d6", "c71585", "db7093",
            # Purples
            "8a2be2", "9932cc", "8b008b", "4b0082", "6a0dad", "ba55d3", "9370db",
            "7b68ee", "9966cc", "aa00ff", "8000ff",
            # Cyans and electric blues
            "00ffff", "0000ff", "1e90ff", "00bfff", "4169e1", "0080ff",
            "0066ff", "3366ff", "6600ff"
        ]

        normalized_colors = self._normalize_hex_colors(colors)
        return self._match_color_indicators(normalized_colors, cyberfemme_indicators)

    def _normalize_hex_colors(self, colors: List[str]) -> List[str]:
        """Normalize hex colors by removing # and converting to lowercase."""
        normalized = []
        for color in colors:
            normalized_color = color.lower().replace("#", "").strip()
            if len(normalized_color) == 6:  # Valid hex color
                normalized.append(normalized_color)
        return normalized

    def _match_color_indicators(self, colors: List[str], indicators: List[str]) -> bool:
        """Check if any colors match the indicator patterns."""
        for color in colors:
            for indicator in indicators:
                if self._colors_match(color, indicator):
                    return True
        return False

    def _colors_match(self, color1: str, color2: str) -> bool:
        """Check if two hex colors match exactly or are similar."""
        if color1 == color2:
            return True

        # Check for close matches (allow minor variations)
        if len(color1) == 6 and len(color2) == 6:
            try:
                r1, g1, b1 = int(color1[0:2], 16), int(color1[2:4], 16), int(color1[4:6], 16)
                r2, g2, b2 = int(color2[0:2], 16), int(color2[2:4], 16), int(color2[4:6], 16)

                # Calculate color distance
                distance = ((r1-r2)**2 + (g1-g2)**2 + (b1-b2)**2)**0.5
                return distance < 40  # Allow small variations
            except ValueError:
                return False

        return False

    def _has_earth_tones(self, colors: List[str]) -> bool:
        """Check if color palette contains earth tones."""
        earth_indicators = [
            "8b4513", "a0522d", "cd853f", "daa520", "b8860b", "d2691e",
            "f4a460", "deb887", "bc8f8f", "f5deb3", "2e8b57", "228b22",
            "556b2f", "6b8e23", "808000", "8fbc8f"
        ]

        color_str = " ".join(colors).lower()
        return any(indicator in color_str for indicator in earth_indicators)

    def _has_tech_colors(self, colors: List[str]) -> bool:
        """Check if color palette contains tech/minimal colors (blacks, whites, blues, grays)."""
        tech_indicators = [
            "000000", "ffffff", "808080", "c0c0c0", "696969", "2f4f4f",
            "1a1a1a", "0080ff", "0066cc", "003366", "191970", "00008b"
        ]

        color_str = " ".join(colors).lower()
        return any(indicator in color_str for indicator in tech_indicators)

    def _has_vintage_colors(self, colors: List[str]) -> bool:
        """Check if color palette contains vintage/retro colors."""
        vintage_indicators = [
            "b8860b", "cd853f", "daa520", "f5deb3", "ffe4b5", "ffdab9",
            "8b4513", "a0522d", "d2691e", "ff6347", "dc143c", "b22222"
        ]

        color_str = " ".join(colors).lower()
        return any(indicator in color_str for indicator in vintage_indicators)

    def _generate_color_palettes(self, color_data: Dict, theme_name: str) -> Tuple[List[str], List[str]]:
        """Generate primary and accent color palettes based on theme."""
        dominant_colors = color_data["dominant_colors"]

        if not dominant_colors:
            # Theme-specific fallback palettes
            if theme_name == "cyberfemme":
                return (
                    ["#1a1a2e", "#16213e", "#0f3460"],
                    ["#ff00ff", "#00ffff", "#ff1493", "#9932cc"]
                )
            elif theme_name == "organic":
                return (
                    ["#2e4a3d", "#4a6741", "#6b8e23"],
                    ["#8fbc8f", "#daa520", "#cd853f"]
                )
            elif theme_name == "tech":
                return (
                    ["#1a1a1a", "#2f2f2f", "#404040"],
                    ["#0080ff", "#00bfff", "#ffffff"]
                )
            else:  # vintage or adaptive
                return (
                    ["#3d2f1f", "#5d4037", "#795548"],
                    ["#daa520", "#cd853f", "#b8860b"]
                )

        # Use actual detected colors as primary palette (first 3-5)
        primary_colors = dominant_colors[:4]

        # Generate accent colors based on theme
        accent_colors = self._generate_accent_colors(theme_name, color_data)

        return primary_colors, accent_colors

    def _generate_accent_colors(self, theme_name: str, color_data: Dict) -> List[str]:
        """Generate accent colors that complement the theme."""
        if theme_name == "cyberfemme":
            return ["#ff00ff", "#00ffff", "#ff1493", "#9370db"]
        elif theme_name == "organic":
            return ["#8fbc8f", "#daa520", "#f4a460", "#90ee90"]
        elif theme_name == "tech":
            return ["#0080ff", "#00bfff", "#ffffff", "#c0c0c0"]
        elif theme_name == "vintage":
            return ["#daa520", "#cd853f", "#f5deb3", "#ffe4b5"]
        else:  # adaptive
            # Use complementary colors from detected palette
            return color_data["dominant_colors"][4:8] if len(color_data["dominant_colors"]) > 4 else ["#ffffff"]

    def _get_fallback_theme(self) -> ThemeProfile:
        """Return a fallback theme when analysis fails."""
        return ThemeProfile(
            theme_name="adaptive",
            confidence=0.5,
            primary_colors=["#1a1a2e", "#16213e", "#0f3460"],
            accent_colors=["#ffffff", "#c0c0c0", "#808080"],
            temperature_bias="neutral",
            mood_profile={"balanced": 1.0},
            energy_level="medium",
            complexity_preference=0.5,
            harmony_types=["balanced"]
        )

class ContentThemeCache:
    """
    Cache for content theme analysis to avoid repeated computation.
    """

    def __init__(self, cache_file: str = "config/theme_cache.json"):
        """Initialize the theme cache."""
        self.cache_file = Path(cache_file)
        self.cache: Dict[str, Dict] = {}
        self.load_cache()

    def load_cache(self) -> None:
        """Load cached theme data from file."""
        try:
            if self.cache_file.exists():
                with open(self.cache_file, 'r') as f:
                    self.cache = json.load(f)
                logger.debug(f"Loaded theme cache with {len(self.cache)} entries")
        except (json.JSONDecodeError, IOError) as e:
            logger.warning(f"Failed to load theme cache: {str(e)}")
            self.cache = {}

    def save_cache(self) -> None:
        """Save current cache to file."""
        try:
            self.cache_file.parent.mkdir(parents=True, exist_ok=True)
            with open(self.cache_file, 'w') as f:
                json.dump(self.cache, f, indent=2)
            logger.debug("Theme cache saved successfully")
        except IOError as e:
            logger.error(f"Failed to save theme cache: {str(e)}")

    def get_theme(self, collection_hash: str) -> Optional[ThemeProfile]:
        """Get cached theme for a collection."""
        if collection_hash in self.cache:
            data = self.cache[collection_hash]
            return ThemeProfile(
                theme_name=data["theme_name"],
                confidence=data["confidence"],
                primary_colors=data["primary_colors"],
                accent_colors=data["accent_colors"],
                temperature_bias=data["temperature_bias"],
                mood_profile=data["mood_profile"],
                energy_level=data["energy_level"],
                complexity_preference=data["complexity_preference"],
                harmony_types=data["harmony_types"]
            )
        return None

    def set_theme(self, collection_hash: str, theme: ThemeProfile) -> None:
        """Cache a theme for a collection."""
        self.cache[collection_hash] = theme.to_dict()
        self.save_cache()
