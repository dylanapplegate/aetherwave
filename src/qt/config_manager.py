"""
Aetherwave Configuration Manager - Settings and preferences for Qt frontend
Handles theme configuration, display settings, and user preferences.
"""

import yaml
import logging
from typing import Any, Dict, Optional
from pathlib import Path


class ConfigManager:
    """Manages configuration for the Aetherwave Qt frontend."""
    
    def __init__(self, config_path: Optional[Path] = None):
        self.logger = logging.getLogger(__name__)
        
        # Use default config path if none provided
        if config_path is None:
            config_path = Path(__file__).parent.parent.parent / "config" / "config.yaml"
        
        self.config_path = config_path
        self.config_data: Dict[str, Any] = {}
        self.load_config()
    
    def load_config(self) -> None:
        """Load configuration from YAML file."""
        try:
            if self.config_path.exists():
                with open(self.config_path, 'r') as f:
                    self.config_data = yaml.safe_load(f) or {}
                self.logger.info(f"Loaded configuration from {self.config_path}")
            else:
                self.logger.warning(f"Config file not found: {self.config_path}")
                self.config_data = self._get_default_config()
                self.save_config()
        except Exception as e:
            self.logger.error(f"Failed to load config: {e}")
            self.config_data = self._get_default_config()
    
    def save_config(self) -> None:
        """Save current configuration to YAML file."""
        try:
            # Ensure config directory exists
            self.config_path.parent.mkdir(parents=True, exist_ok=True)
            
            with open(self.config_path, 'w') as f:
                yaml.dump(self.config_data, f, default_flow_style=False, indent=2)
            self.logger.info(f"Saved configuration to {self.config_path}")
        except Exception as e:
            self.logger.error(f"Failed to save config: {e}")
    
    def get(self, key: str, default: Any = None) -> Any:
        """Get a configuration value using dot notation (e.g., 'api.base_url')."""
        keys = key.split('.')
        value = self.config_data
        
        try:
            for k in keys:
                value = value[k]
            return value
        except (KeyError, TypeError):
            return default
    
    def set(self, key: str, value: Any) -> None:
        """Set a configuration value using dot notation."""
        keys = key.split('.')
        config = self.config_data
        
        # Navigate to the parent of the target key
        for k in keys[:-1]:
            if k not in config:
                config[k] = {}
            config = config[k]
        
        # Set the final value
        config[keys[-1]] = value
        self.logger.debug(f"Set config {key} = {value}")
    
    def _get_default_config(self) -> Dict[str, Any]:
        """Get default configuration values."""
        return {
            'api': {
                'base_url': 'http://localhost:8000',
                'timeout': 30,
                'health_check_interval': 30
            },
            'display': {
                'fullscreen': True,
                'multi_monitor': True,
                'vsync': True,
                'target_fps': 60,
                'transition_duration': 1.0,
                'image_duration': 5.0
            },
            'effects': {
                'bloom_enabled': True,
                'bloom_intensity': 0.3,
                'chromatic_aberration': True,
                'aberration_strength': 0.02,
                'glitch_enabled': True,
                'glitch_probability': 0.1,
                'scan_lines': True,
                'noise_overlay': True
            },
            'theme': {
                'cyberfemme': {
                    'primary_color': [255, 0, 255],  # Magenta
                    'secondary_color': [0, 255, 255],  # Cyan
                    'accent_color': [255, 255, 255],  # White
                    'background_color': [0, 0, 0],  # Black
                    'ui_transparency': 0.8
                }
            },
            'input': {
                'space_next_image': True,
                'arrow_navigation': True,
                'escape_exit': True,
                'f_fullscreen_toggle': True,
                'r_refresh': True
            },
            'debug': {
                'show_fps': False,
                'show_debug_overlay': False,
                'log_level': 'INFO'
            }
        }
    
    def get_theme_config(self, theme_name: str = 'cyberfemme') -> Dict[str, Any]:
        """Get theme-specific configuration."""
        return self.get(f'theme.{theme_name}', {})
    
    def get_display_config(self) -> Dict[str, Any]:
        """Get display-related configuration."""
        return self.get('display', {})
    
    def get_effects_config(self) -> Dict[str, Any]:
        """Get visual effects configuration."""
        return self.get('effects', {})
    
    def is_fullscreen(self) -> bool:
        """Check if fullscreen mode is enabled."""
        return self.get('display.fullscreen', True)
    
    def is_multi_monitor(self) -> bool:
        """Check if multi-monitor mode is enabled."""
        return self.get('display.multi_monitor', True)
    
    def get_transition_duration(self) -> float:
        """Get image transition duration in seconds."""
        return self.get('display.transition_duration', 1.0)
    
    def get_image_duration(self) -> float:
        """Get how long each image is displayed in seconds."""
        return self.get('display.image_duration', 5.0)
    
    def get_target_fps(self) -> int:
        """Get target frame rate."""
        return self.get('display.target_fps', 60)