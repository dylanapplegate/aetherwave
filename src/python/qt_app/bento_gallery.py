"""
Bento Gallery Window (Qt)
Displays images in a bento box layout using QGridLayout and BentoOptimizer assignments.
"""
from typing import List, Dict, Any, Optional
from PySide6.QtWidgets import QWidget, QGridLayout, QLabel, QApplication, QSizePolicy
from PySide6.QtGui import QPixmap
from PySide6.QtCore import Qt, QSize
import math
import os
import sys

# Import BentoOptimizer from API
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../api')))
from bento_optimizer import bento_optimizer


class ResizeImageLabel(QLabel):
    """QLabel that scales its pixmap to available size while keeping aspect ratio."""
    def __init__(self, pixmap: Optional[QPixmap] = None, parent: Optional[QWidget] = None):
        super().__init__(parent)
        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self._orig: Optional[QPixmap] = pixmap
        if pixmap is not None:
            self.setPixmap(pixmap)

    def setPixmap(self, pm: QPixmap) -> None:  # type: ignore[override]
        self._orig = pm
        super().setPixmap(self._scaled_for_current_size())

    def resizeEvent(self, event) -> None:  # noqa: D401
        """On resize, rescale pixmap to new label size."""
        if self._orig is not None:
            super().setPixmap(self._scaled_for_current_size())
        super().resizeEvent(event)

    def _scaled_for_current_size(self) -> QPixmap:
        if self._orig is None or self._orig.isNull():
            return QPixmap()
        target = QSize(max(1, self.width() - 2), max(1, self.height() - 2))
        return self._orig.scaled(
            target,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation,
        )


class BentoGalleryWindow(QWidget):
    """
    Responsive bento layout:
    - Compute rows/cols from window size
    - Tile the optimizer's pattern across the whole grid
    - Backfill remaining cells to avoid gaps
    """

    def __init__(self, image_metadatas: List[Dict[str, Any]], pattern_name: str = "balanced_mix"):
        super().__init__()
        self.setWindowTitle("Aetherwave Bento Gallery")
        self.grid = QGridLayout()
        self.setLayout(self.grid)
        self._image_metadatas = image_metadatas
        self._pattern_name = pattern_name
        self._apply_common_styling()
        self._populate_bento_grid()

    # --------------------------- layout helpers --------------------------- #
    def _apply_common_styling(self) -> None:
        base = max(6, min(14, max(1, self.width()) // 140))
        self.grid.setSpacing(base)
        self.grid.setContentsMargins(base * 2, base * 2, base * 2, base * 2)
        self.setStyleSheet("background-color: #2c3e50;")

    def _target_cell_px(self) -> int:
        # Derive a target cell from available area so density scales with monitor size
        w = max(1, self.width())
        h = max(1, self.height())
        area = w * h
        # Aim for ~90–140 tiles total depending on size (dense but readable)
        target_tiles = max(48, min(140, area // (140 * 140)))  # ~140px tile baseline
        # Derive approximate tile width from target tile count and aspect ratio
        aspect = w / h if h else 1.0
        cols_guess = max(8, int((target_tiles * aspect) ** 0.5))
        tile_w = max(110, min(220, w // max(1, cols_guess)))
        return tile_w

    def _compute_rows_cols(self) -> tuple[int, int]:
        margins = self.grid.contentsMargins()
        spacing = self.grid.spacing()
        usable_w = max(1, self.width() - (margins.left() + margins.right()))
        usable_h = max(1, self.height() - (margins.top() + margins.bottom()))
        cell = self._target_cell_px()
        # Bias columns by aspect ratio for ultra-wide monitors
        aspect = usable_w / usable_h if usable_h else 1.0
        base_cols = max(8, int(usable_w / (cell + spacing)))
        base_rows = max(6, int(usable_h / (cell + spacing)))
        if aspect > 2.2:
            base_cols = int(base_cols * min(1.6, aspect / 1.6))
        # Clamp to avoid over/under density; ensure at least 96 cells on big displays
        total = base_cols * base_rows
        if total < 96 and usable_w > 1800:
            base_cols = max(base_cols, 14)
            base_rows = max(base_rows, 8)
        return base_rows, base_cols

    def _clear_grid(self) -> None:
        while self.grid.count():
            item = self.grid.takeAt(0)
            w = item.widget()
            if w:
                w.setParent(None)

    def _get_all_paths(self) -> List[str]:
        paths: List[str] = []
        for md in self._image_metadatas:
            p = md.get("filename") or md.get("path")
            if isinstance(p, str) and os.path.exists(p):
                paths.append(p)
        return paths

    def _populate_bento_grid(self) -> None:
        # Re-apply adaptive spacing for current size
        self._apply_common_styling()
        self._clear_grid()

        result = bento_optimizer.optimize_layout(self._pattern_name, self._image_metadatas)
        assignments = result.get("assignments", {})
        pattern = next((p for p in bento_optimizer.patterns if p.name == self._pattern_name), None)
        if not pattern:
            return

        rows, cols = self._compute_rows_cols()
        occupied = [[False for _ in range(cols)] for _ in range(rows)]

        # Base pattern size
        pat_rows = max(s.row + s.height for s in pattern.slots)
        pat_cols = max(s.col + s.width for s in pattern.slots)
        tiles_r = max(1, math.ceil(rows / pat_rows))
        tiles_c = max(1, math.ceil(cols / pat_cols))

        # Prepare iterator over images; start with those used by assignments
        paths = self._get_all_paths()
        if not paths:
            paths = []
        # Try to maintain the optimizer-chosen ordering first
        chosen = [a.get("filename") for k, a in sorted(assignments.items()) if isinstance(a, dict) and a.get("filename")]
        remaining = [p for p in paths if p not in chosen]
        ordered_paths = (chosen + remaining) if paths else []
        img_idx = 0

        # Helper to create label by path
        def make_label(path: Optional[str]) -> QLabel:
            if not path:
                lab = QLabel("")
                lab.setAlignment(Qt.AlignmentFlag.AlignCenter)
                return lab
            pm = QPixmap(path)
            if pm.isNull():
                lab = QLabel(f"Missing: {os.path.basename(path)}")
                lab.setAlignment(Qt.AlignmentFlag.AlignCenter)
                return lab
            return ResizeImageLabel(pm)

        # Tile the pattern across the grid
        for tr in range(tiles_r):
            for tc in range(tiles_c):
                row_off = tr * pat_rows
                col_off = tc * pat_cols
                for slot in pattern.slots:
                    r = slot.row + row_off
                    c = slot.col + col_off
                    if r >= rows or c >= cols:
                        continue
                    rh = min(slot.height, rows - r)
                    cw = min(slot.width, cols - c)

                    # Use assignment for first tile only, otherwise iterate images
                    path: Optional[str] = None
                    if tr == 0 and tc == 0:
                        a = assignments.get(f"slot_{slot.row}_{slot.col}")
                        if isinstance(a, dict):
                            path = a.get("filename")
                    if not path and ordered_paths:
                        path = ordered_paths[img_idx % len(ordered_paths)]
                        img_idx += 1

                    label = make_label(path)
                    self.grid.addWidget(label, r, c, rh, cw)
                    for rr in range(r, r + rh):
                        for cc in range(c, c + cw):
                            occupied[rr][cc] = True

        # Backfill any remaining single cells
        for r in range(rows):
            for c in range(cols):
                if occupied[r][c]:
                    continue
                path = ordered_paths[img_idx % len(ordered_paths)] if ordered_paths else None
                img_idx += 1 if ordered_paths else 0
                self.grid.addWidget(make_label(path), r, c, 1, 1)

        # Stretch to fill
        for r in range(rows):
            self.grid.setRowStretch(r, 1)
        for c in range(cols):
            self.grid.setColumnStretch(c, 1)

    # --------------------------- Qt events --------------------------- #
    def resizeEvent(self, event) -> None:  # noqa: D401
        self._populate_bento_grid()
        super().resizeEvent(event)


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
