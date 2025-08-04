"""
Aetherwave Python Classification Service

This module provides the FastAPI service for image metadata classification
and analysis. It includes endpoints for processing artwork metadata and
serving classification results to the C++ display engine.
"""

from typing import Dict, List, Optional, Any
from fastapi import FastAPI, HTTPException, BackgroundTasks
from pydantic import BaseModel
import json
import logging
from pathlib import Path
import aiofiles
from advanced_classifier import AdvancedImageClassifier

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Constants
METADATA_DIR = "config/metadata"

app = FastAPI(
    title="Aetherwave Classification Service",
    description="Advanced image metadata classification and analysis for Aetherwave display engine",
    version="2.0.0"
)

# Initialize the advanced classifier
classifier = AdvancedImageClassifier()

class ImageMetadata(BaseModel):
    """
    Enhanced Pydantic model for comprehensive image metadata.
    """
    # Basic info
    filename: str
    width: int
    height: int
    aspect_ratio: float
    format: Optional[str] = None
    megapixels: Optional[float] = None

    # Color analysis
    dominant_color: str
    color_palette: List[str]
    color_temperature: str
    color_harmony: str
    brightness: float
    saturation: float

    # Complexity analysis
    edge_density: float
    texture_complexity: float
    color_complexity: float
    overall_complexity: float

    # Mood analysis
    primary_mood: str
    emotional_tone: str
    energy_level: str
    mood_confidence: float

    # Display recommendations
    recommended_duration: float
    cinematic_score: float

    # Metadata
    classification_confidence: float
    analyzer_version: str
    processing_timestamp: str

class ClassificationRequest(BaseModel):
    """
    Enhanced request model for image classification.
    """
    image_path: str
    include_colors: bool = True
    include_mood: bool = True
    include_complexity: bool = True
    save_metadata: bool = True

class ClassificationResponse(BaseModel):
    """
    Enhanced response model for classification results.
    """
    ok: bool
    metadata: Optional[ImageMetadata] = None
    error: Optional[str] = None
    processing_time: Optional[float] = None

class BatchClassificationRequest(BaseModel):
    """
    Request model for batch classification of multiple images.
    """
    image_directory: str
    file_extensions: List[str] = [".jpg", ".jpeg", ".png", ".tiff", ".webp"]
    max_images: int = 100

class BatchClassificationResponse(BaseModel):
    """
    Response model for batch classification results.
    """
    ok: bool
    processed_count: int
    failed_count: int
    results: List[Dict[str, Any]]
    errors: List[str]

@app.get("/")
async def root() -> Dict[str, Any]:
    """
    Root endpoint for service health check.

    Returns:
        Dict containing service status and version info.
    """
    return {
        "service": "Aetherwave Classification Service",
        "version": "2.0.0",
        "status": "running",
        "features": [
            "advanced_color_analysis",
            "complexity_analysis",
            "mood_detection",
            "batch_processing",
            "cinematic_scoring"
        ]
    }

@app.get("/health")
async def health_check() -> Dict[str, Any]:
    """
    Comprehensive health check endpoint for monitoring.

    Returns:
        Dict indicating service health status and system info.
    """
    try:
        # Test classifier functionality
        test_result = classifier.color_analyzer._get_fallback_colors()

        return {
            "healthy": True,
            "service_version": "2.0.0",
            "classifier_ready": bool(test_result),
            "features_available": {
                "color_analysis": True,
                "complexity_analysis": True,
                "mood_analysis": True,
                "opencv_available": True
            }
        }
    except Exception as e:
        logger.error(f"Health check failed: {str(e)}")
        return {
            "healthy": False,
            "error": str(e)
        }

@app.post("/classify", response_model=ClassificationResponse)
async def classify_image(request: ClassificationRequest) -> ClassificationResponse:
    """
    Classify an image using advanced computer vision and color analysis.

    Args:
        request: Classification request with image path and options.

    Returns:
        ClassificationResponse with comprehensive metadata or error information.
    """
    import time
    start_time = time.time()

    try:
        logger.info(f"Advanced classification requested for: {request.image_path}")

        # Validate image path
        image_path = Path(request.image_path)
        if not image_path.exists():
            raise FileNotFoundError(f"Image file not found: {request.image_path}")

        # Perform advanced classification
        classification_result = classifier.classify_image(str(image_path))

        # Extract data for response model
        basic_info = classification_result["basic_info"]
        color_analysis = classification_result["color_analysis"]
        complexity_analysis = classification_result["complexity_analysis"]
        mood_analysis = classification_result["mood_analysis"]
        metadata = classification_result["classification_metadata"]

        # Create enhanced metadata object
        enhanced_metadata = ImageMetadata(
            # Basic info
            filename=basic_info["filename"],
            width=basic_info["width"],
            height=basic_info["height"],
            aspect_ratio=basic_info["aspect_ratio"],
            format=basic_info.get("format"),
            megapixels=basic_info.get("megapixels"),

            # Color analysis
            dominant_color=color_analysis["dominant_color"],
            color_palette=color_analysis["palette"],
            color_temperature=color_analysis["temperature"],
            color_harmony=color_analysis["harmony_type"],
            brightness=color_analysis["brightness"],
            saturation=color_analysis["saturation"],

            # Complexity analysis
            edge_density=complexity_analysis["edge_density"],
            texture_complexity=complexity_analysis["texture_complexity"],
            color_complexity=complexity_analysis["color_complexity"],
            overall_complexity=complexity_analysis["overall_complexity"],

            # Mood analysis
            primary_mood=mood_analysis["primary_mood"],
            emotional_tone=mood_analysis["emotional_tone"],
            energy_level=mood_analysis["energy_level"],
            mood_confidence=mood_analysis["confidence"],

            # Display recommendations
            recommended_duration=metadata["recommended_display_duration"],
            cinematic_score=metadata["cinematic_score"],

            # Metadata
            classification_confidence=metadata["confidence_score"],
            analyzer_version=metadata["analyzer_version"],
            processing_timestamp=metadata["processing_timestamp"]
        )

        # Save metadata if requested
        if request.save_metadata:
            await save_classification_metadata(image_path.name, classification_result)

        processing_time = time.time() - start_time
        logger.info(f"Classification completed in {processing_time:.2f}s for: {request.image_path}")

        return ClassificationResponse(
            ok=True,
            metadata=enhanced_metadata,
            processing_time=round(processing_time, 3)
        )

    except FileNotFoundError as e:
        logger.error(f"File not found: {str(e)}")
        return ClassificationResponse(
            ok=False,
            error=f"File not found: {str(e)}",
            processing_time=round(time.time() - start_time, 3)
        )
    except Exception as e:
        logger.error(f"Classification error: {str(e)}")
        return ClassificationResponse(
            ok=False,
            error=str(e),
            processing_time=round(time.time() - start_time, 3)
        )

@app.post("/classify/batch", response_model=BatchClassificationResponse)
async def classify_batch(
    request: BatchClassificationRequest,
    background_tasks: BackgroundTasks
) -> BatchClassificationResponse:
    """
    Classify multiple images in a directory.

    Args:
        request: Batch classification request with directory and options.
        background_tasks: FastAPI background tasks for async processing.

    Returns:
        BatchClassificationResponse with processing summary.
    """
    try:
        logger.info(f"Batch classification requested for directory: {request.image_directory}")

        directory = Path(request.image_directory)
        if not directory.exists() or not directory.is_dir():
            raise FileNotFoundError(f"Directory not found: {request.image_directory}")

        # Find image files
        image_files = []
        for ext in request.file_extensions:
            pattern = f"*{ext.lower()}"
            image_files.extend(directory.glob(pattern))
            # Also check uppercase
            pattern = f"*{ext.upper()}"
            image_files.extend(directory.glob(pattern))

        # Limit number of images
        image_files = image_files[:request.max_images]

        results = []
        errors = []
        processed_count = 0
        failed_count = 0

        # Process each image
        for image_file in image_files:
            try:
                classification_result = classifier.classify_image(str(image_file))
                results.append({
                    "filename": image_file.name,
                    "classification": classification_result,
                    "status": "success"
                })
                processed_count += 1

                # Save metadata in background
                background_tasks.add_task(
                    save_classification_metadata,
                    image_file.name,
                    classification_result
                )

            except Exception as e:
                error_msg = f"Failed to process {image_file.name}: {str(e)}"
                logger.error(error_msg)
                errors.append(error_msg)
                failed_count += 1

        logger.info(f"Batch processing complete: {processed_count} processed, {failed_count} failed")

        return BatchClassificationResponse(
            ok=True,
            processed_count=processed_count,
            failed_count=failed_count,
            results=results,
            errors=errors
        )

    except Exception as e:
        logger.error(f"Batch classification error: {str(e)}")
        return BatchClassificationResponse(
            ok=False,
            processed_count=0,
            failed_count=0,
            results=[],
            errors=[str(e)]
        )

@app.get("/metadata/{filename}")
async def get_metadata(filename: str) -> Dict[str, Any]:
    """
    Retrieve cached metadata for a specific image.

    Args:
        filename: Name of the image file.

    Returns:
        Cached metadata dictionary or error.
    """
    try:
        metadata_file = Path(METADATA_DIR) / f"{filename}.json"

        if not metadata_file.exists():
            raise HTTPException(
                status_code=404,
                detail=f"Metadata not found for {filename}"
            )

        async with aiofiles.open(metadata_file, "r") as f:
            content = await f.read()
            metadata = json.loads(content)

        return {
            "filename": filename,
            "cached": True,
            "metadata": metadata,
            "cache_file": str(metadata_file)
        }

    except json.JSONDecodeError as e:
        raise HTTPException(
            status_code=500,
            detail=f"Invalid metadata file for {filename}: {str(e)}"
        )
    except Exception as e:
        raise HTTPException(
            status_code=500,
            detail=f"Error retrieving metadata: {str(e)}"
        )

@app.get("/analytics/summary")
async def get_analytics_summary() -> Dict[str, Any]:
    """
    Get analytics summary of processed images.

    Returns:
        Summary statistics of processed images.
    """
    try:
        metadata_dir = Path(METADATA_DIR)
        if not metadata_dir.exists():
            return {"total_processed": 0, "summary": "No processed images found"}

        metadata_files = list(metadata_dir.glob("*.json"))

        # Basic statistics
        total_processed = len(metadata_files)

        # Analyze cached metadata for trends
        mood_counts = {}
        complexity_scores = []
        cinematic_scores = []

        for metadata_file in metadata_files[:50]:  # Sample first 50 for performance
            try:
                async with aiofiles.open(metadata_file, "r") as f:
                    content = await f.read()
                    data = json.loads(content)

                # Extract analytics data
                mood = data.get("mood_analysis", {}).get("primary_mood", "unknown")
                mood_counts[mood] = mood_counts.get(mood, 0) + 1

                complexity = data.get("complexity_analysis", {}).get("overall_complexity", 0)
                complexity_scores.append(complexity)

                cinematic = data.get("classification_metadata", {}).get("cinematic_score", 0)
                cinematic_scores.append(cinematic)

            except (json.JSONDecodeError, KeyError):
                continue

        # Calculate averages
        avg_complexity = sum(complexity_scores) / len(complexity_scores) if complexity_scores else 0
        avg_cinematic = sum(cinematic_scores) / len(cinematic_scores) if cinematic_scores else 0

        return {
            "total_processed": total_processed,
            "analytics": {
                "mood_distribution": mood_counts,
                "average_complexity": round(avg_complexity, 3),
                "average_cinematic_score": round(avg_cinematic, 3),
                "sample_size": len(complexity_scores)
            }
        }

    except Exception as e:
        logger.error(f"Analytics summary error: {str(e)}")
        raise HTTPException(
            status_code=500,
            detail=f"Error generating analytics: {str(e)}"
        )

async def save_classification_metadata(filename: str, classification_data: Dict[str, Any]) -> None:
    """
    Save classification metadata to cache file.

    Args:
        filename: Name of the image file
        classification_data: Complete classification results
    """
    try:
        # Create metadata directory
        metadata_dir = Path(METADATA_DIR)
        metadata_dir.mkdir(parents=True, exist_ok=True)

        # Save metadata
        metadata_file = metadata_dir / f"{filename}.json"
        async with aiofiles.open(metadata_file, "w") as f:
            await f.write(json.dumps(classification_data, indent=2))

        logger.debug(f"Metadata saved: {metadata_file}")

    except Exception as e:
        logger.error(f"Failed to save metadata for {filename}: {str(e)}")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host="127.0.0.1",
        port=8000,
        reload=True,
        log_level="info"
    )
