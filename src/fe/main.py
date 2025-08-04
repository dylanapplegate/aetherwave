#!/usr/bin/env python3
"""
Aetherwave Qt Frontend - Main Application Entry Point
A cyberfemme gallery with multi-monitor support and OpenGL effects.
"""

import sys
import logging
from typing import Optional
from pathlib import Path
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QFont

from gallery_window import GalleryWindow
from api_client import AetherwaveAPIClient
from config_manager import ConfigManager


def setup_logging() -> None:
    """Configure logging for the Qt application."""
    log_dir = Path("build/logs")
    log_dir.mkdir(parents=True, exist_ok=True)
    
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
        handlers=[
            logging.FileHandler(log_dir / "aetherwave_qt.log"),
            logging.StreamHandler(sys.stdout)
        ]
    )


def setup_application() -> QApplication:
    """Configure the Qt application with high-DPI support."""
    # Enable high-DPI support for retina displays
    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
    QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)
    
    app = QApplication(sys.argv)
    app.setApplicationName("Aetherwave")
    app.setApplicationVersion("2.0.0")
    app.setOrganizationName("Aetherwave")
    
    # Set cyberfemme-inspired font
    font = QFont("Menlo", 12)  # Monospace for cyberpunk aesthetic
    app.setFont(font)
    
    return app


class AetherwaveQtApp:
    """Main application coordinator for the Qt frontend."""
    
    def __init__(self):
        self.logger = logging.getLogger(__name__)
        self.app: Optional[QApplication] = None
        self.gallery_window: Optional[GalleryWindow] = None
        self.api_client: Optional[AetherwaveAPIClient] = None
        self.config: Optional[ConfigManager] = None
        
    def initialize(self) -> bool:
        """Initialize all application components."""
        try:
            # Setup logging
            setup_logging()
            self.logger.info("Starting Aetherwave Qt Frontend")
            
            # Load configuration
            self.config = ConfigManager()
            
            # Setup Qt application
            self.app = setup_application()
            
            # Initialize API client
            self.api_client = AetherwaveAPIClient(
                base_url=self.config.get('api.base_url', 'http://localhost:8000')
            )
            
            # Verify API connectivity
            if not self.api_client.health_check():
                self.logger.error("Failed to connect to Aetherwave API")
                self.logger.error("Please ensure the Python FastAPI backend is running")
                return False
            
            # Create main gallery window
            self.gallery_window = GalleryWindow(
                api_client=self.api_client,
                config=self.config,
                window_id=1  # Main window gets ID 1
            )
            
            # Setup periodic health checks
            self.setup_health_monitoring()
            
            self.logger.info("Aetherwave Qt Frontend initialized successfully")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to initialize application: {e}")
            return False
    
    def setup_health_monitoring(self) -> None:
        """Setup periodic API health checks."""
        health_timer = QTimer()
        health_timer.timeout.connect(self.check_api_health)
        health_timer.start(30000)  # Check every 30 seconds
        
    def check_api_health(self) -> None:
        """Periodic health check for the API backend."""
        if not self.api_client.health_check():
            self.logger.warning("API health check failed")
            if self.gallery_window:
                self.gallery_window.show_connection_error()
    
    def run(self) -> int:
        """Run the application main loop."""
        if not self.initialize():
            return 1
        
        # Show main window
        self.gallery_window.show()
        self.gallery_window.start_gallery()
        
        # Run Qt event loop
        return self.app.exec()


def main() -> int:
    """Application entry point."""
    app = AetherwaveQtApp()
    return app.run()


if __name__ == '__main__':
    sys.exit(main())