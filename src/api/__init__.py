"""
Aetherwave Python Classification Module

This module provides image classification and metadata analysis
capabilities for the Aetherwave media display engine.
"""

__version__ = "1.0.0"
__author__ = "Aetherwave Project"

from .main import app, ClassificationRequest, ClassificationResponse, ImageMetadata

__all__ = [
    "app",
    "ClassificationRequest",
    "ClassificationResponse",
    "ImageMetadata"
]
