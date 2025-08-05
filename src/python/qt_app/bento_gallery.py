"""
Bento Gallery Window (Qt)
Displays images in a bento box layout using QGridLayout and BentoOptimizer assignments.
"""
from typing import List, Dict, Any
from PySide6.QtWidgets import QWidget, QGridLayout, QLabel, QApplication
from PySide6.QtGui import QPixmap
from PySide6.QtCore import Qt
import os
import sys

# Import BentoOptimizer from API
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../api')))
from bento_optimizer import bento_optimizer

class BentoGalleryWindow(QWidget):
    """
    Qt window that displays images in a bento box layout using BentoOptimizer slot assignments.
    """
    def __init__(self, image_metadatas: List[Dict[str, Any]], pattern_name: str = "balanced_mix"):
        super().__init__()
        self.setWindowTitle("Aetherwave Bento Gallery")
        self.grid = QGridLayout()
        self.setLayout(self.grid)
        self._populate_bento_grid(image_metadatas, pattern_name)

    def _populate_bento_grid(self, image_metadatas: List[Dict[str, Any]], pattern_name: str):
        result = bento_optimizer.optimize_layout(pattern_name, image_metadatas)
        assignments = result.get("assignments", {})
        pattern = next((p for p in bento_optimizer.patterns if p.name == pattern_name), None)
        if not pattern:
            return
        slot_map = {f"slot_{slot.row}_{slot.col}": slot for slot in pattern.slots}
        # Fill all slots, even if not assigned
        for slot in pattern.slots:
            slot_key = f"slot_{slot.row}_{slot.col}"
            assignment = assignments.get(slot_key)
            label = QLabel()
            if assignment:
                image_path = assignment["filename"]
                pixmap = QPixmap(image_path)
                if pixmap.isNull():
                    label.setText(f"Missing: {os.path.basename(image_path)}")
                else:
                    label.setPixmap(pixmap.scaled(200 * slot.width, 200 * slot.height, Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation))
            else:
                label.setText("[Empty]")
            label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.grid.addWidget(label, slot.row, slot.col, slot.height, slot.width)
        # Set row/col stretch to fill window
        max_row = max(slot.row + slot.height for slot in pattern.slots)
        max_col = max(slot.col + slot.width for slot in pattern.slots)
        for r in range(max_row):
            self.grid.setRowStretch(r, 1)
        for c in range(max_col):
            self.grid.setColumnStretch(c, 1)
        self.grid.setSpacing(12)
        self.grid.setContentsMargins(24, 24, 24, 24)
        # Set background color for visual cohesion
        self.setStyleSheet("background-color: #2c3e50;")

if __name__ == "__main__":
    # Example usage: load metadata from config/sample_metadata.json
    import json
    sample_metadata_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../config/sample_metadata.json'))
    with open(sample_metadata_path, "r") as f:
        image_metadatas = json.load(f)
    app = QApplication(sys.argv)
    window = BentoGalleryWindow(image_metadatas, pattern_name="balanced_mix")
    window.resize(1200, 800)
    window.show()
    sys.exit(app.exec())
