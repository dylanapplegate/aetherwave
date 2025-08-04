#!/usr/bin/env python3
"""
Aetherwave Content-Driven Aesthetic Demo

This script demonstrates how Aetherwave automatically analyzes image content
and generates appropriate aesthetic themes without manual configuration.
"""

import json
import sys
import tempfile
from pathlib import Path

# Add src path for imports
sys.path.append(str(Path(__file__).parent.parent / "src" / "python"))

from content_theme_analyzer import CollectionAnalyzer, ThemeProfile
from advanced_classifier import AdvancedImageClassifier

def create_demo_collection_metadata():
    """Create sample metadata that represents different art styles."""

    collections = {
        "cyberfemme": [
            {
                "filename": "neon_portrait_01.jpg",
                "color_analysis": {
                    "dominant_color": "#ff00ff",
                    "palette": ["#ff00ff", "#9932cc", "#00ffff", "#da70d6"],
                    "temperature": "cool",
                    "harmony_type": "triadic",
                    "brightness": 0.6,
                    "saturation": 0.85
                },
                "mood_analysis": {
                    "primary_mood": "vibrant",
                    "emotional_tone": "uplifting",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.8
                }
            },
            {
                "filename": "cyber_fashion_02.jpg",
                "color_analysis": {
                    "dominant_color": "#9370db",
                    "palette": ["#9370db", "#ff1493", "#00bfff", "#8a2be2"],
                    "temperature": "cool",
                    "harmony_type": "complementary",
                    "brightness": 0.5,
                    "saturation": 0.9
                },
                "mood_analysis": {
                    "primary_mood": "cinematic",
                    "emotional_tone": "intense",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.7
                }
            },
            {
                "filename": "trans_colors_03.jpg",
                "color_analysis": {
                    "dominant_color": "#ff69b4",
                    "palette": ["#ff69b4", "#87ceeb", "#ffffff", "#ff1493"],
                    "temperature": "cool",
                    "harmony_type": "analogous",
                    "brightness": 0.7,
                    "saturation": 0.75
                },
                "mood_analysis": {
                    "primary_mood": "vibrant",
                    "emotional_tone": "uplifting",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.6
                }
            }
        ],

        "organic_nature": [
            {
                "filename": "forest_landscape_01.jpg",
                "color_analysis": {
                    "dominant_color": "#2e8b57",
                    "palette": ["#2e8b57", "#8fbc8f", "#556b2f", "#daa520"],
                    "temperature": "warm",
                    "harmony_type": "analogous",
                    "brightness": 0.4,
                    "saturation": 0.4
                },
                "mood_analysis": {
                    "primary_mood": "peaceful",
                    "emotional_tone": "calm",
                    "energy_level": "low"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.5
                }
            },
            {
                "filename": "mountain_sunset_02.jpg",
                "color_analysis": {
                    "dominant_color": "#cd853f",
                    "palette": ["#cd853f", "#daa520", "#f4a460", "#8b4513"],
                    "temperature": "warm",
                    "harmony_type": "monochromatic",
                    "brightness": 0.6,
                    "saturation": 0.3
                },
                "mood_analysis": {
                    "primary_mood": "serene",
                    "emotional_tone": "contemplative",
                    "energy_level": "medium"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.4
                }
            }
        ],

        "tech_minimal": [
            {
                "filename": "architecture_01.jpg",
                "color_analysis": {
                    "dominant_color": "#2f4f4f",
                    "palette": ["#2f4f4f", "#708090", "#ffffff", "#000000"],
                    "temperature": "cool",
                    "harmony_type": "monochromatic",
                    "brightness": 0.3,
                    "saturation": 0.1
                },
                "mood_analysis": {
                    "primary_mood": "dramatic",
                    "emotional_tone": "contemplative",
                    "energy_level": "medium"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.8
                }
            },
            {
                "filename": "urban_geometry_02.jpg",
                "color_analysis": {
                    "dominant_color": "#191970",
                    "palette": ["#191970", "#4169e1", "#87ceeb", "#ffffff"],
                    "temperature": "cool",
                    "harmony_type": "triadic",
                    "brightness": 0.2,
                    "saturation": 0.6
                },
                "mood_analysis": {
                    "primary_mood": "dynamic",
                    "emotional_tone": "intense",
                    "energy_level": "high"
                },
                "complexity_analysis": {
                    "overall_complexity": 0.9
                }
            }
        ]
    }

    return collections

def analyze_collection_style(collection_name: str, metadata_list: list) -> ThemeProfile:
    """Analyze a collection and determine its aesthetic theme."""

    print(f"\n🎨 Analyzing '{collection_name}' collection...")
    print(f"   Found {len(metadata_list)} images")

    # Create temporary directory with metadata files
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        # Save metadata files
        for i, metadata in enumerate(metadata_list):
            json_file = temp_path / f"image_{i+1}.json"
            with open(json_file, 'w') as f:
                json.dump(metadata, f, indent=2)

        # Analyze collection theme
        analyzer = CollectionAnalyzer()
        theme = analyzer.analyze_collection_theme(str(temp_path))

        return theme

def print_theme_analysis(theme: ThemeProfile):
    """Print detailed theme analysis results."""

    print(f"   🎯 Detected Theme: {theme.theme_name.upper()} (confidence: {theme.confidence:.2f})")
    print(f"   🌡️  Temperature Bias: {theme.temperature_bias}")
    print(f"   ⚡ Energy Level: {theme.energy_level}")
    print(f"   🎨 Primary Colors: {', '.join(theme.primary_colors[:3])}")
    print(f"   ✨ Accent Colors: {', '.join(theme.accent_colors[:3])}")
    print(f"   🎭 Top Moods: {', '.join([mood for mood, weight in sorted(theme.mood_profile.items(), key=lambda x: x[1], reverse=True)[:3]])}")
    print(f"   🔧 Complexity Preference: {theme.complexity_preference:.2f}")

def demonstrate_adaptive_ui_application(theme: ThemeProfile):
    """Show how theme would be applied to UI components."""

    print(f"\n   🖥️  UI Application for {theme.theme_name.upper()} theme:")

    # Background color selection
    bg_color = theme.primary_colors[0] if theme.primary_colors else "#1a1a1a"
    print(f"      • Background: {bg_color} (primary color)")

    # Accent color selection
    accent_color = theme.accent_colors[0] if theme.accent_colors else "#ffffff"
    print(f"      • Accents: {accent_color} (for highlights/UI elements)")

    # Transition style based on energy
    if theme.energy_level == "high":
        transition_style = "glitch wipes, quick fades (0.8s)"
    elif theme.energy_level == "medium":
        transition_style = "smooth slides, medium fades (1.2s)"
    else:
        transition_style = "slow dissolves, gentle fades (2.0s)"
    print(f"      • Transitions: {transition_style}")

    # Effects based on theme
    if theme.theme_name == "cyberfemme":
        effects = "bloom, chromatic aberration, neon glows"
    elif theme.theme_name == "organic":
        effects = "soft glows, subtle vignettes, warm gradients"
    elif theme.theme_name == "tech":
        effects = "sharp edges, high contrast, minimal shadows"
    else:
        effects = "balanced lighting, subtle effects"
    print(f"      • Visual Effects: {effects}")

def main():
    """Run the content-driven aesthetic demonstration."""

    print("🌟 Aetherwave Content-Driven Aesthetic Demo")
    print("=" * 50)
    print()
    print("This demo shows how Aetherwave automatically analyzes image content")
    print("and generates appropriate aesthetic themes without manual configuration.")
    print()
    print("🔍 The system detects:")
    print("   • Dominant color palettes and temperature bias")
    print("   • Mood patterns (vibrant, serene, dramatic, etc.)")
    print("   • Visual complexity and energy levels")
    print("   • Color harmony types (monochromatic, triadic, etc.)")
    print()
    print("🎨 Then automatically maps content patterns to aesthetic themes:")
    print("   • Purple/pink/blue + high saturation + vibrant moods → Cyberfemme")
    print("   • Earth tones + warm colors + peaceful moods → Organic")
    print("   • High contrast + cool colors + dramatic moods → Tech/Minimal")

    # Get demo collections
    collections = create_demo_collection_metadata()

    # Analyze each collection
    for collection_name, metadata_list in collections.items():
        theme = analyze_collection_style(collection_name, metadata_list)
        print_theme_analysis(theme)
        demonstrate_adaptive_ui_application(theme)
        print()

    print("✨ Key Benefits of Content-Driven Aesthetics:")
    print("   • Authentic visual expression (themes emerge from actual content)")
    print("   • Universal reusability (works with any art collection)")
    print("   • Zero manual configuration (automatic adaptation)")
    print("   • Intelligent evolution (adapts as new content is added)")
    print()
    print("🎯 For your cyberfemme collection:")
    print("   The system will detect the purple/pink/blue palette, high saturation,")
    print("   and vibrant/cinematic moods - automatically generating cyberfemme")
    print("   aesthetics without hardcoding them!")
    print()
    print("🔮 Try this with your actual images:")
    print("   1. Run batch classification on your cyberfemme collection")
    print("   2. Call /analyze/collection-theme API endpoint")
    print("   3. Watch as cyberfemme aesthetics emerge from content analysis!")

if __name__ == "__main__":
    main()
