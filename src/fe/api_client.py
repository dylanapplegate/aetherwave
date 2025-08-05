"""
Aetherwave API Client - HTTP interface to the Python FastAPI backend
Handles image retrieval, theme analysis, and collection management.
"""

import requests
import logging
from typing import Dict, List, Optional, Any
from pathlib import Path
import time


class AetherwaveAPIClient:
    """Client for communicating with the Aetherwave FastAPI backend."""

    def __init__(self, base_url: str = "http://localhost:8000"):
        self.base_url = base_url.rstrip('/')
        self.session = requests.Session()
        self.session.timeout = (5.0, 30.0)  # (connect, read) timeout
        self.logger = logging.getLogger(__name__)

        # Cache for frequently accessed data
        self._image_list_cache: Optional[List[str]] = None
        self._theme_cache: Optional[Dict[str, Any]] = None
        self._cache_timestamp = 0
        self._cache_ttl = 300  # 5 minutes

    def health_check(self) -> bool:
        """Check if the API backend is healthy and responsive."""
        try:
            response = self.session.get(f"{self.base_url}/health")
            response.raise_for_status()

            health_data = response.json()
            is_healthy = health_data.get('healthy', False)

            if is_healthy:
                service_version = health_data.get('service_version', 'unknown')
                self.logger.info(f"API health check passed (version: {service_version})")
            else:
                self.logger.warning("API reports unhealthy status")

            return is_healthy

        except requests.RequestException as e:
            self.logger.error(f"API health check failed: {e}")
            return False
        except Exception as e:
            self.logger.error(f"Unexpected error during health check: {e}")
            return False

    def get_image_list(self, force_refresh: bool = False) -> List[str]:
        """Get list of available images from the API."""
        # Check cache first
        if not force_refresh and self._image_list_cache and self._is_cache_valid():
            return self._image_list_cache

        try:
            response = self.session.get(f"{self.base_url}/images/list")
            response.raise_for_status()

            data = response.json()
            image_list = data.get('images', [])

            # Update cache
            self._image_list_cache = image_list
            self._cache_timestamp = time.time()

            self.logger.info(f"Retrieved {len(image_list)} images from API")
            return image_list

        except requests.RequestException as e:
            self.logger.error(f"Failed to get image list: {e}")
            return self._image_list_cache or []
        except Exception as e:
            self.logger.error(f"Unexpected error getting image list: {e}")
            return self._image_list_cache or []

    def get_image_url(self, filename: str) -> str:
        """Get the full URL for an image file."""
        return f"{self.base_url}/images/{filename}"

    def get_collection_theme(self, sample_size: int = 5, force_refresh: bool = False) -> Dict[str, Any]:
        """Get the current collection theme analysis."""
        # Check cache first
        if not force_refresh and self._theme_cache and self._is_cache_valid():
            return self._theme_cache

        try:
            payload = {
                "collection_path": "assets/images",
                "sample_size": sample_size
            }

            response = self.session.post(
                f"{self.base_url}/analyze/collection-theme",
                json=payload
            )
            response.raise_for_status()

            theme_data = response.json()

            # Update cache
            self._theme_cache = theme_data
            self._cache_timestamp = time.time()

            theme_name = theme_data.get('theme', {}).get('theme_name', 'unknown')
            confidence = theme_data.get('theme', {}).get('confidence', 0)

            self.logger.info(f"Retrieved theme: {theme_name} (confidence: {confidence:.2f})")
            return theme_data

        except requests.RequestException as e:
            self.logger.error(f"Failed to get collection theme: {e}")
            return self._theme_cache or {}
        except Exception as e:
            self.logger.error(f"Unexpected error getting collection theme: {e}")
            return self._theme_cache or {}

    def classify_image(self, filename: str, include_metadata: bool = True) -> Dict[str, Any]:
        """Get classification data for a specific image."""
        try:
            payload = {
                "image_path": f"assets/images/{filename}",
                "include_metadata": include_metadata
            }

            response = self.session.post(
                f"{self.base_url}/classify",
                json=payload
            )
            response.raise_for_status()

            classification_data = response.json()
            self.logger.debug(f"Classified image {filename}")
            return classification_data

        except requests.RequestException as e:
            self.logger.error(f"Failed to classify image {filename}: {e}")
            return {}
        except Exception as e:
            self.logger.error(f"Unexpected error classifying image {filename}: {e}")
            return {}

    def _is_cache_valid(self) -> bool:
        """Check if the current cache is still valid."""
        return (time.time() - self._cache_timestamp) < self._cache_ttl

    def clear_cache(self) -> None:
        """Clear all cached data."""
        self._image_list_cache = None
        self._theme_cache = None
        self._cache_timestamp = 0
        self.logger.info("API cache cleared")

    def get_api_stats(self) -> Dict[str, Any]:
        """Get API statistics and performance metrics."""
        try:
            response = self.session.get(f"{self.base_url}/stats")
            response.raise_for_status()
            return response.json()

        except requests.RequestException as e:
            self.logger.error(f"Failed to get API stats: {e}")
            return {}
        except Exception as e:
            self.logger.error(f"Unexpected error getting API stats: {e}")
            return {}

    def get_image_pixmap(self, filename: str) -> 'QPixmap':
        """Get image as QPixmap for display in Qt widgets."""
        from PySide6.QtGui import QPixmap
        import requests

        try:
            response = self.session.get(self.get_image_url(filename))
            response.raise_for_status()

            # Create QPixmap from image data
            pixmap = QPixmap()
            if pixmap.loadFromData(response.content):
                return pixmap
            else:
                self.logger.warning(f"Failed to load pixmap data for {filename}")
                return QPixmap()  # Return empty pixmap

        except requests.RequestException as e:
            self.logger.error(f"Failed to fetch image {filename}: {e}")
            return QPixmap()  # Return empty pixmap
        except Exception as e:
            self.logger.error(f"Unexpected error loading pixmap for {filename}: {e}")
            return QPixmap()  # Return empty pixmap