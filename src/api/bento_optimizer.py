"""
Bento Layout Optimizer

Analyzes image aspect ratios to create optimal bento box arrangements
that minimize cropping and maximize visual harmony.
"""

from typing import List, Dict, Any, Tuple, Optional
import logging
from dataclasses import dataclass
from enum import Enum
import random

logger = logging.getLogger(__name__)


class AspectCategory(Enum):
    SQUARE = "square"        # ~1:1 (0.9-1.1)
    PORTRAIT = "portrait"    # Tall (< 0.9)
    LANDSCAPE = "landscape"  # Wide (> 1.1)
    PANORAMIC = "panoramic"  # Very wide (> 1.8)
    VERTICAL = "vertical"    # Very tall (< 0.6)


@dataclass
class ImageAnalysis:
    filename: str
    width: int
    height: int
    aspect_ratio: float
    category: AspectCategory
    crop_tolerance: float  # How much cropping this image can tolerate (0-1)


@dataclass
class TileSlot:
    row: int
    col: int
    width: int  # Grid spans
    height: int  # Grid spans
    aspect_ratio: float
    preferred_categories: List[AspectCategory]
    importance: float  # 0-1, higher = more prominent


@dataclass
class BentoPattern:
    name: str
    description: str
    slots: List[TileSlot]
    ideal_for: List[AspectCategory]


class BentoOptimizer:
    """Optimizes bento layouts based on image aspect ratios."""
    
    def __init__(self):
        self.patterns = self._create_bento_patterns()
    
    def _create_bento_patterns(self) -> List[BentoPattern]:
        """Create bento patterns optimized for different aspect ratio distributions."""
        return [
            # Portrait-heavy collection
            BentoPattern(
                name="portrait_showcase",
                description="Optimized for portrait/vertical images",
                slots=[
                    TileSlot(0, 0, 2, 3, 0.67, [AspectCategory.PORTRAIT], 1.0),  # Large portrait
                    TileSlot(0, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),     # Small square
                    TileSlot(1, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),     # Small square
                    TileSlot(2, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),     # Small square
                    TileSlot(0, 3, 2, 1, 2.0, [AspectCategory.LANDSCAPE], 0.8),  # Wide strip
                    TileSlot(1, 3, 2, 2, 1.0, [AspectCategory.PORTRAIT], 0.9),   # Medium portrait
                ],
                ideal_for=[AspectCategory.PORTRAIT, AspectCategory.VERTICAL]
            ),
            
            # Landscape-heavy collection
            BentoPattern(
                name="landscape_gallery",
                description="Optimized for landscape/wide images",
                slots=[
                    TileSlot(0, 0, 1, 3, 3.0, [AspectCategory.PANORAMIC], 1.0),  # Wide hero
                    TileSlot(1, 0, 2, 2, 1.0, [AspectCategory.LANDSCAPE], 0.9),  # Large landscape
                    TileSlot(1, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),     # Square accent
                    TileSlot(2, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),     # Square accent
                    TileSlot(3, 0, 1, 2, 2.0, [AspectCategory.LANDSCAPE], 0.8),  # Medium wide
                    TileSlot(3, 2, 1, 1, 1.0, [AspectCategory.PORTRAIT], 0.6),   # Small portrait
                ],
                ideal_for=[AspectCategory.LANDSCAPE, AspectCategory.PANORAMIC]
            ),
            
            # Mixed collection
            BentoPattern(
                name="balanced_mix",
                description="Balanced layout for mixed aspect ratios",
                slots=[
                    TileSlot(0, 0, 2, 2, 1.0, [AspectCategory.SQUARE, AspectCategory.LANDSCAPE], 1.0),
                    TileSlot(0, 2, 1, 2, 0.5, [AspectCategory.PORTRAIT], 0.8),
                    TileSlot(2, 0, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),
                    TileSlot(2, 1, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),
                    TileSlot(2, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.6),
                    TileSlot(1, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),
                ],
                ideal_for=[AspectCategory.SQUARE]
            ),
            
            # Square-focused
            BentoPattern(
                name="grid_harmony",
                description="Perfect for square and near-square images",
                slots=[
                    TileSlot(0, 0, 2, 2, 1.0, [AspectCategory.SQUARE], 1.0),     # Large square
                    TileSlot(0, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.8),     # Medium square
                    TileSlot(1, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.8),     # Medium square
                    TileSlot(2, 0, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),     # Small square
                    TileSlot(2, 1, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),     # Small square
                    TileSlot(2, 2, 1, 1, 1.0, [AspectCategory.SQUARE], 0.7),     # Small square
                ],
                ideal_for=[AspectCategory.SQUARE]
            )
        ]
    
    def analyze_image(self, metadata: Dict[str, Any]) -> ImageAnalysis:
        """Analyze a single image's aspect ratio characteristics."""
        try:
            width = metadata.get('width', 1)
            height = metadata.get('height', 1)
            aspect_ratio = width / height
            
            # Categorize aspect ratio
            if 0.9 <= aspect_ratio <= 1.1:
                category = AspectCategory.SQUARE
                crop_tolerance = 0.8  # Squares handle cropping well
            elif aspect_ratio < 0.6:
                category = AspectCategory.VERTICAL
                crop_tolerance = 0.3  # Very tall images are sensitive to cropping
            elif aspect_ratio < 0.9:
                category = AspectCategory.PORTRAIT
                crop_tolerance = 0.5  # Moderate cropping tolerance
            elif aspect_ratio > 1.8:
                category = AspectCategory.PANORAMIC
                crop_tolerance = 0.4  # Wide images sensitive to height cropping
            else:
                category = AspectCategory.LANDSCAPE
                crop_tolerance = 0.6  # Good cropping tolerance
            
            return ImageAnalysis(
                filename=metadata.get('filename', ''),
                width=width,
                height=height,
                aspect_ratio=aspect_ratio,
                category=category,
                crop_tolerance=crop_tolerance
            )
            
        except Exception as e:
            logger.warning(f"Failed to analyze image metadata: {e}")
            # Return default square analysis
            return ImageAnalysis(
                filename=metadata.get('filename', ''),
                width=1, height=1, aspect_ratio=1.0,
                category=AspectCategory.SQUARE,
                crop_tolerance=0.8
            )
    
    def analyze_collection(self, image_metadatas: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Analyze a collection of images to determine optimal bento patterns."""
        if not image_metadatas:
            return {"error": "No images to analyze"}
        
        # Analyze each image
        analyses = [self.analyze_image(metadata) for metadata in image_metadatas]
        
        # Count categories
        category_counts = {}
        total_crop_tolerance = 0
        
        for analysis in analyses:
            category = analysis.category.value
            category_counts[category] = category_counts.get(category, 0) + 1
            total_crop_tolerance += analysis.crop_tolerance
        
        # Calculate percentages
        total_images = len(analyses)
        category_percentages = {
            cat: count / total_images for cat, count in category_counts.items()
        }
        
        avg_crop_tolerance = total_crop_tolerance / total_images
        
        # Determine best patterns
        recommended_patterns = self._recommend_patterns(category_percentages)
        
        return {
            "collection_size": total_images,
            "aspect_distribution": category_percentages,
            "average_crop_tolerance": avg_crop_tolerance,
            "recommended_patterns": recommended_patterns,
            "analysis_details": [
                {
                    "filename": a.filename,
                    "aspect_ratio": round(a.aspect_ratio, 2),
                    "category": a.category.value,
                    "crop_tolerance": round(a.crop_tolerance, 2)
                }
                for a in analyses[:10]  # First 10 for brevity
            ]
        }
    
    def _recommend_patterns(self, category_percentages: Dict[str, float]) -> List[str]:
        """Recommend bento patterns based on aspect ratio distribution."""
        recommendations = []
        
        # Thresholds for pattern recommendations
        if category_percentages.get('portrait', 0) > 0.5:
            recommendations.append('portrait_showcase')
        
        if category_percentages.get('landscape', 0) + category_percentages.get('panoramic', 0) > 0.5:
            recommendations.append('landscape_gallery')
        
        if category_percentages.get('square', 0) > 0.6:
            recommendations.append('grid_harmony')
        
        # Always include balanced mix as fallback
        if not recommendations or len(set(category_percentages.values())) > 1:
            recommendations.append('balanced_mix')
        
        return recommendations[:3]  # Max 3 recommendations
    
    def optimize_layout(self, pattern_name: str, image_metadatas: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Create optimal image-to-tile assignments for a given pattern."""
        pattern = next((p for p in self.patterns if p.name == pattern_name), None)
        if not pattern:
            return {"error": f"Pattern '{pattern_name}' not found"}
        
        # Analyze images
        analyses = [self.analyze_image(metadata) for metadata in image_metadatas]
        
        # Sort slots by importance (descending)
        sorted_slots = sorted(pattern.slots, key=lambda s: s.importance, reverse=True)
        
        # Assign images to slots
        assignments = {}
        used_images = set()
        
        for slot in sorted_slots:
            best_image = self._find_best_image_for_slot(slot, analyses, used_images)
            if best_image:
                assignments[f"slot_{slot.row}_{slot.col}"] = {
                    "filename": best_image.filename,
                    "aspect_match": abs(best_image.aspect_ratio - slot.aspect_ratio),
                    "crop_impact": max(0, 1 - best_image.crop_tolerance)
                }
                used_images.add(best_image.filename)
        
        return {
            "pattern": pattern_name,
            "assignments": assignments,
            "total_slots": len(pattern.slots),
            "filled_slots": len(assignments),
            "optimization_score": self._calculate_optimization_score(assignments)
        }
    
    def _find_best_image_for_slot(self, slot: TileSlot, analyses: List[ImageAnalysis], 
                                  used_images: set) -> Optional[ImageAnalysis]:
        """Find the best image for a specific tile slot."""
        available_images = [a for a in analyses if a.filename not in used_images]
        if not available_images:
            return None
        
        # Score each image for this slot
        scored_images = []
        for image in available_images:
            score = 0
            
            # Aspect ratio match (higher is better)
            aspect_diff = abs(image.aspect_ratio - slot.aspect_ratio)
            aspect_score = max(0, 1 - aspect_diff)  # 1 = perfect match, 0 = very different
            score += aspect_score * 0.5
            
            # Category preference
            if image.category in slot.preferred_categories:
                score += 0.3
            
            # Crop tolerance for this slot size
            crop_score = image.crop_tolerance
            score += crop_score * 0.2
            
            scored_images.append((score, image))
        
        # Return best scoring image
        scored_images.sort(key=lambda x: x[0], reverse=True)
        return scored_images[0][1] if scored_images else None
    
    def _calculate_optimization_score(self, assignments: Dict[str, Any]) -> float:
        """Calculate overall optimization score for a layout."""
        if not assignments:
            return 0.0
        
        total_score = 0
        for assignment in assignments.values():
            # Lower aspect match difference is better
            aspect_score = max(0, 1 - assignment["aspect_match"])
            # Lower crop impact is better
            crop_score = max(0, 1 - assignment["crop_impact"])
            total_score += (aspect_score + crop_score) / 2
        
        return total_score / len(assignments)


# Global optimizer instance
bento_optimizer = BentoOptimizer()
