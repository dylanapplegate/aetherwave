"""
Tests for Aetherwave metadata classification functionality.

This module contains pytest tests for the FastAPI application
and classification endpoints.
"""

import pytest
import json
from pathlib import Path
from fastapi.testclient import TestClient
from typing import Dict, Any

# Import the FastAPI app
import sys
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "src" / "python"))

from main import app, ClassificationRequest, ClassificationResponse, ImageMetadata

# Create test client
client = TestClient(app)

class TestAetherwaveAPI:
    """Test cases for Aetherwave FastAPI endpoints."""
    
    def test_root_endpoint(self) -> None:
        """Test the root endpoint returns service information."""
        response = client.get("/")
        assert response.status_code == 200
        
        data = response.json()
        assert "service" in data
        assert "version" in data
        assert "status" in data
        assert data["service"] == "Aetherwave Classification Service"
        assert data["version"] == "1.0.0"
        assert data["status"] == "running"
    
    def test_health_check(self) -> None:
        """Test the health check endpoint."""
        response = client.get("/health")
        assert response.status_code == 200
        
        data = response.json()
        assert "healthy" in data
        assert data["healthy"] is True
    
    def test_classify_endpoint_success(self) -> None:
        """Test successful image classification."""
        request_data = {
            "image_path": "test_image.jpg",
            "include_colors": True,
            "include_mood": True
        }
        
        response = client.post("/classify", json=request_data)
        assert response.status_code == 200
        
        data = response.json()
        assert "ok" in data
        assert data["ok"] is True
        assert "metadata" in data
        
        # Validate metadata structure
        metadata = data["metadata"]
        assert "filename" in metadata
        assert "width" in metadata
        assert "height" in metadata
        assert "dominant_colors" in metadata
        assert "mood" in metadata
        assert "complexity" in metadata
        assert "classification_confidence" in metadata
        
        # Validate specific values from stub implementation
        assert metadata["filename"] == "test_image.jpg"
        assert metadata["width"] == 1920
        assert metadata["height"] == 1080
        assert metadata["mood"] == "cinematic"
        assert isinstance(metadata["dominant_colors"], list)
        assert len(metadata["dominant_colors"]) == 3
    
    def test_classify_endpoint_minimal_request(self) -> None:
        """Test classification with minimal request parameters."""
        request_data = {
            "image_path": "minimal_test.jpg"
        }
        
        response = client.post("/classify", json=request_data)
        assert response.status_code == 200
        
        data = response.json()
        assert data["ok"] is True
        assert data["metadata"]["filename"] == "minimal_test.jpg"
    
    def test_get_metadata_endpoint(self) -> None:
        """Test the metadata retrieval endpoint."""
        filename = "test_artwork.jpg"
        response = client.get(f"/metadata/{filename}")
        assert response.status_code == 200
        
        data = response.json()
        assert "filename" in data
        assert "cached" in data
        assert "metadata" in data
        assert data["filename"] == filename
        assert data["cached"] is True
        
        # Validate nested metadata structure
        metadata = data["metadata"]
        assert "dominant_colors" in metadata
        assert "mood" in metadata
        assert "complexity" in metadata
    
    def test_classification_request_model(self) -> None:
        """Test the ClassificationRequest Pydantic model."""
        # Test with all fields
        request = ClassificationRequest(
            image_path="test.jpg",
            include_colors=True,
            include_mood=False
        )
        assert request.image_path == "test.jpg"
        assert request.include_colors is True
        assert request.include_mood is False
        
        # Test with minimal fields (defaults)
        minimal_request = ClassificationRequest(image_path="minimal.jpg")
        assert minimal_request.image_path == "minimal.jpg"
        assert minimal_request.include_colors is True  # default
        assert minimal_request.include_mood is True    # default
    
    def test_image_metadata_model(self) -> None:
        """Test the ImageMetadata Pydantic model."""
        metadata = ImageMetadata(
            filename="test.jpg",
            width=1920,
            height=1080,
            dominant_colors=["#ff0000", "#00ff00", "#0000ff"],
            mood="dramatic",
            complexity=0.8,
            classification_confidence=0.95
        )
        
        assert metadata.filename == "test.jpg"
        assert metadata.width == 1920
        assert metadata.height == 1080
        assert len(metadata.dominant_colors) == 3
        assert metadata.mood == "dramatic"
        assert metadata.complexity == 0.8
        assert metadata.classification_confidence == 0.95
    
    def test_classification_response_model(self) -> None:
        """Test the ClassificationResponse Pydantic model."""
        # Test successful response
        metadata = ImageMetadata(
            filename="test.jpg",
            width=1920,
            height=1080,
            dominant_colors=["#000000"]
        )
        
        response = ClassificationResponse(ok=True, metadata=metadata)
        assert response.ok is True
        assert response.metadata is not None
        assert response.error is None
        
        # Test error response
        error_response = ClassificationResponse(ok=False, error="Test error")
        assert error_response.ok is False
        assert error_response.metadata is None
        assert error_response.error == "Test error"

class TestConfigurationOutput:
    """Test cases for configuration file generation."""
    
    def test_sample_metadata_generation(self) -> None:
        """Test that classification generates sample metadata file."""
        # Make a classification request
        request_data = {"image_path": "config_test.jpg"}
        response = client.post("/classify", json=request_data)
        
        assert response.status_code == 200
        
        # Check if sample metadata file was created
        config_dir = Path("config")
        metadata_file = config_dir / "sample_metadata.json"
        
        # File should exist (created by the endpoint)
        if metadata_file.exists():
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
            
            # Validate JSON structure
            assert "filename" in metadata
            assert "width" in metadata
            assert "height" in metadata
            assert "dominant_colors" in metadata
            assert "mood" in metadata
            assert "complexity" in metadata
            assert "classification_confidence" in metadata

@pytest.fixture
def temp_config_dir(tmp_path: Path) -> Path:
    """Create a temporary config directory for testing."""
    config_dir = tmp_path / "config"
    config_dir.mkdir()
    return config_dir

class TestSchemaValidation:
    """Test cases for validating against configuration schema."""
    
    def test_metadata_schema_compliance(self) -> None:
        """Test that generated metadata complies with expected schema."""
        request_data = {"image_path": "schema_test.jpg"}
        response = client.post("/classify", json=request_data)
        
        assert response.status_code == 200
        metadata = response.json()["metadata"]
        
        # Required fields
        required_fields = [
            "filename", "width", "height", "dominant_colors"
        ]
        for field in required_fields:
            assert field in metadata, f"Required field '{field}' missing"
        
        # Type validation
        assert isinstance(metadata["width"], int)
        assert isinstance(metadata["height"], int)
        assert isinstance(metadata["dominant_colors"], list)
        assert isinstance(metadata["filename"], str)
        
        # Optional fields type validation
        if metadata.get("mood"):
            assert isinstance(metadata["mood"], str)
        if metadata.get("complexity"):
            assert isinstance(metadata["complexity"], (int, float))
            assert 0 <= metadata["complexity"] <= 1
        if metadata.get("classification_confidence"):
            assert isinstance(metadata["classification_confidence"], (int, float))
            assert 0 <= metadata["classification_confidence"] <= 1

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
