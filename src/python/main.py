"""
Aetherwave Python Classification Service

This module provides the FastAPI service for image metadata classification
and analysis. It includes endpoints for processing artwork metadata and
serving classification results to the C++ display engine.
"""

from typing import Dict, List, Optional, Any
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import json
import logging
from pathlib import Path

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(
    title="Aetherwave Classification Service",
    description="Image metadata classification and analysis for Aetherwave display engine",
    version="1.0.0"
)

class ImageMetadata(BaseModel):
    """
    Pydantic model for image metadata structure.
    """
    filename: str
    width: int
    height: int
    dominant_colors: List[str]
    mood: Optional[str] = None
    complexity: Optional[float] = None
    classification_confidence: Optional[float] = None

class ClassificationRequest(BaseModel):
    """
    Request model for image classification.
    """
    image_path: str
    include_colors: bool = True
    include_mood: bool = True

class ClassificationResponse(BaseModel):
    """
    Response model for classification results.
    """
    ok: bool
    metadata: Optional[ImageMetadata] = None
    error: Optional[str] = None

@app.get("/")
async def root() -> Dict[str, str]:
    """
    Root endpoint for service health check.
    
    Returns:
        Dict containing service status and version info.
    """
    return {
        "service": "Aetherwave Classification Service", 
        "version": "1.0.0",
        "status": "running"
    }

@app.get("/health")
async def health_check() -> Dict[str, bool]:
    """
    Health check endpoint for monitoring.
    
    Returns:
        Dict indicating service health status.
    """
    return {"healthy": True}

@app.post("/classify", response_model=ClassificationResponse)
async def classify_image(request: ClassificationRequest) -> ClassificationResponse:
    """
    Classify an image and return metadata.
    
    This is a stub implementation that returns sample metadata.
    Future implementation will use colorthief, OpenCV, and other
    libraries for actual image analysis.
    
    Args:
        request: Classification request with image path and options.
        
    Returns:
        ClassificationResponse with metadata or error information.
    """
    try:
        logger.info(f"Classification requested for: {request.image_path}")
        
        # Stub implementation - return sample metadata
        sample_metadata = ImageMetadata(
            filename=Path(request.image_path).name,
            width=1920,
            height=1080,
            dominant_colors=["#1a1a2e", "#16213e", "#0f3460"],
            mood="cinematic",
            complexity=0.7,
            classification_confidence=0.95
        )
        
        # Write sample metadata to config directory
        config_dir = Path("config")
        config_dir.mkdir(exist_ok=True)
        
        metadata_file = config_dir / "sample_metadata.json"
        with open(metadata_file, "w") as f:
            json.dump(sample_metadata.dict(), f, indent=2)
        
        logger.info(f"Sample metadata written to: {metadata_file}")
        
        return ClassificationResponse(
            ok=True,
            metadata=sample_metadata
        )
        
    except Exception as e:
        logger.error(f"Classification error: {str(e)}")
        return ClassificationResponse(
            ok=False,
            error=str(e)
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
        # Future: Load from actual metadata cache
        # For now, return sample data
        return {
            "filename": filename,
            "cached": True,
            "metadata": {
                "dominant_colors": ["#1a1a2e", "#16213e", "#0f3460"],
                "mood": "cinematic",
                "complexity": 0.7
            }
        }
    except Exception as e:
        raise HTTPException(status_code=404, detail=f"Metadata not found: {str(e)}")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app", 
        host="127.0.0.1", 
        port=8000, 
        reload=True,
        log_level="info"
    )
