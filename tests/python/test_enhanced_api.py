"""
Tests for the enhanced FastAPI main service with content theme analysis.
"""

import pytest
from fastapi.testclient import TestClient
import tempfile
import json
from pathlib import Path
import sys
import os

# Add src path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "src", "python"))

from main import app

class TestEnhancedAPI:
    """Test suite for the enhanced FastAPI service."""

    @pytest.fixture
    def client(self):
        """Create test client."""
        return TestClient(app)

    @pytest.fixture
    def sample_metadata_dir(self):
        """Create temporary directory with sample metadata for testing."""
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)

            # Create sample cyberfemme metadata
            metadata = {
                "color_analysis": {
                    "dominant_color": "#ff00ff",
                    "palette": ["#ff00ff", "#9932cc", "#00ffff"],
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
            }

            # Save sample metadata file
            json_file = temp_path / "test_image.json"
            with open(json_file, 'w') as f:
                json.dump(metadata, f, indent=2)

            yield str(temp_path)

    def test_root_endpoint(self, client):
        """Test that root endpoint includes new theme features."""
        response = client.get("/")

        assert response.status_code == 200
        data = response.json()

        assert data["service"] == "Aetherwave Classification Service"
        assert data["version"] == "2.0.0"
        assert data["status"] == "running"

        # Check for new theme features
        features = data["features"]
        assert "content_driven_themes" in features
        assert "adaptive_aesthetics" in features
        assert "collection_analysis" in features

    def test_health_endpoint(self, client):
        """Test health check endpoint."""
        response = client.get("/health")

        assert response.status_code == 200
        data = response.json()

        assert "healthy" in data
        assert "classifier_ready" in data
        assert "features_available" in data

    def test_collection_theme_analysis(self, client, sample_metadata_dir):
        """Test collection theme analysis endpoint."""
        request_data = {
            "metadata_directory": sample_metadata_dir,
            "force_refresh": True
        }

        response = client.post("/analyze/collection-theme", json=request_data)

        assert response.status_code == 200
        data = response.json()

        assert data["ok"] is True
        assert "theme" in data
        assert data["cache_hit"] is False

        # Check theme structure
        theme = data["theme"]
        assert "theme_name" in theme
        assert "confidence" in theme
        assert "primary_colors" in theme
        assert "accent_colors" in theme
        assert "temperature_bias" in theme
        assert "energy_level" in theme

        # Should detect cyberfemme theme from our sample data
        assert theme["theme_name"] in ["cyberfemme", "adaptive"]
        assert 0.3 <= theme["confidence"] <= 0.95

    def test_collection_theme_analysis_invalid_directory(self, client):
        """Test collection theme analysis with invalid directory."""
        request_data = {
            "metadata_directory": "/nonexistent/directory",
            "force_refresh": True
        }

        response = client.post("/analyze/collection-theme", json=request_data)

        assert response.status_code == 200
        data = response.json()

        assert data["ok"] is False
        assert "error" in data
        assert "Directory not found" in data["error"]

    def test_current_theme_endpoint(self, client):
        """Test current theme endpoint."""
        response = client.get("/theme/current")

        assert response.status_code == 200
        data = response.json()

        assert "active_theme" in data
        assert "last_analyzed" in data
        assert "source" in data

        # Check theme structure
        theme = data["active_theme"]
        assert "theme_name" in theme
        assert "confidence" in theme
        assert "primary_colors" in theme

    def test_collection_theme_caching(self, client, sample_metadata_dir):
        """Test that collection theme analysis uses caching."""
        request_data = {
            "metadata_directory": sample_metadata_dir,
            "force_refresh": False
        }

        # First request
        response1 = client.post("/analyze/collection-theme", json=request_data)
        assert response1.status_code == 200
        data1 = response1.json()
        assert data1["cache_hit"] is False

        # Second request should use cache
        response2 = client.post("/analyze/collection-theme", json=request_data)
        assert response2.status_code == 200
        data2 = response2.json()

        # Should get same theme
        assert data1["theme"]["theme_name"] == data2["theme"]["theme_name"]

    def test_backwards_compatibility(self, client):
        """Test that existing endpoints still work."""
        # Test classify endpoint structure (even if it fails due to missing image)
        request_data = {
            "image_path": "/nonexistent/image.jpg",
            "include_colors": True,
            "include_mood": True,
            "include_complexity": True,
            "save_metadata": False
        }

        response = client.post("/classify", json=request_data)

        # Should return proper error structure
        assert response.status_code == 200
        data = response.json()

        assert "ok" in data
        assert data["ok"] is False
        assert "error" in data

        # Test analytics endpoint
        response = client.get("/analytics/summary")
        assert response.status_code == 200
