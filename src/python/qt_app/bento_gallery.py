"""
Bento Gallery Window (Qt)
Displays images in a bento box layout using QGridLayout and BentoOptimizer assignments.
"""
from typing import List, Dict, Any, Optional
from PySide6.QtWidgets import QWidget, QGridLayout, QLabel, QApplication, QSizePolicy, QFrame
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
    Responsive bento layout on a single logical canvas:
    - Compute canvas rows/cols from monitor size
    - Scale pattern slots proportionally to fill canvas exactly once
    - No overlaps, no tiling; every cell belongs to exactly one slot
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

    def _canvas_rows_cols(self) -> tuple[int, int]:
        """Compute a single bento canvas size from window dimensions.
        Keeps tiles around 140–180 px and biases columns for ultrawide.
        """
        margins = self.grid.contentsMargins()
        spacing = self.grid.spacing()
        usable_w = max(1, self.width() - (margins.left() + margins.right()))
        usable_h = max(1, self.height() - (margins.top() + margins.bottom()))
        aspect = usable_w / usable_h if usable_h else 1.0
        target = 160  # target tile px
        cols = max(6, int(usable_w / (target + spacing)))
        rows = max(6, int(usable_h / (target + spacing)))
        if aspect > 2.0:
            cols = max(cols, int(rows * aspect))
        return rows, cols

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

    def _scale_slots_to_canvas(self, pattern) -> list[dict]:
        """Scale pattern slots to canvas and correct edges to ensure exact coverage.
        Returns list of dicts with row, col, height, width for runtime grid.
        """
        rows, cols = self._canvas_rows_cols()
        pat_rows = max(s.row + s.height for s in pattern.slots)
        pat_cols = max(s.col + s.width for s in pattern.slots)
        # Initial proportional mapping using float scale
        r_scale = rows / pat_rows
        c_scale = cols / pat_cols
        scaled = []
        for s in pattern.slots:
            r = int(round(s.row * r_scale))
            c = int(round(s.col * c_scale))
            rh = int(round(s.height * r_scale))
            cw = int(round(s.width * c_scale))
            scaled.append({"row": r, "col": c, "height": max(1, rh), "width": max(1, cw)})
        # Normalize bands: ensure everything fits within canvas and adjust last slot in each row/col band
        for sl in scaled:
            sl["row"] = max(0, min(rows - 1, sl["row"]))
            sl["col"] = max(0, min(cols - 1, sl["col"]))
            sl["height"] = max(1, min(rows - sl["row"], sl["height"]))
            sl["width"] = max(1, min(cols - sl["col"], sl["width"]))
        # Resolve overlaps by simple occupancy painting; shrink conflicting spans
        occ = [[-1 for _ in range(cols)] for _ in range(rows)]
        for idx, sl in enumerate(scaled):
            r0, c0, rh, cw = sl["row"], sl["col"], sl["height"], sl["width"]
            # Reduce until fits empty region
            while True:
                conflict = False
                for r in range(r0, min(rows, r0 + rh)):
                    for c in range(c0, min(cols, c0 + cw)):
                        if occ[r][c] != -1:
                            conflict = True
                            break
                    if conflict:
                        break
                if not conflict:
                    break
                # Prefer shrinking along the larger span
                if rh >= cw and rh > 1:
                    rh -= 1
                elif cw > 1:
                    cw -= 1
                else:
                    break
            sl["height"], sl["width"] = rh, cw
            for r in range(r0, min(rows, r0 + rh)):
                for c in range(c0, min(cols, c0 + cw)):
                    occ[r][c] = idx
        # Fill any gaps by assigning to nearest slot (simple Voronoi by Manhattan distance)
        coords = [
            (i, s["row"] + s["height"] / 2.0, s["col"] + s["width"] / 2.0)
            for i, s in enumerate(scaled)
        ]
        for r in range(rows):
            for c in range(cols):
                if occ[r][c] != -1:
                    continue
                # Bind r,c as defaults to avoid late-binding in lambdas
                rr, cc = r, c
                nearest = min(coords, key=lambda t, rr=rr, cc=cc: abs(t[1] - rr) + abs(t[2] - cc))[0]
                occ[r][c] = nearest
        # Convert painted regions back into merged rectangles per slot id
        merged: list[dict] = []
        for i, _ in enumerate(scaled):
            # Find bounding box of cells belonging to slot i
            min_r, min_c, max_r, max_c = rows, cols, -1, -1
            for r in range(rows):
                for c in range(cols):
                    if occ[r][c] == i:
                        min_r = min(min_r, r)
                        min_c = min(min_c, c)
                        max_r = max(max_r, r)
                        max_c = max(max_c, c)
            if max_r >= 0:
                merged.append({
                    "row": min_r,
                    "col": min_c,
                    "height": (max_r - min_r + 1),
                    "width": (max_c - min_c + 1),
                })
        return merged

    def _populate_bento_grid(self) -> None:
        self._apply_common_styling()
        self._clear_grid()

        result = bento_optimizer.optimize_layout(self._pattern_name, self._image_metadatas)
        assignments = result.get("assignments", {})
        pattern = next((p for p in bento_optimizer.patterns if p.name == self._pattern_name), None)
        if not pattern:
            return

        # Scale pattern to the runtime canvas exactly once (macro layout)
        scaled_slots = self._scale_slots_to_canvas(pattern)

        # Collect image paths, prefer optimizer picks first
        paths = self._get_all_paths()
        chosen = [a.get("filename") for _, a in sorted(assignments.items()) if isinstance(a, dict) and a.get("filename")]
        remaining = [p for p in paths if p not in chosen]
        ordered_paths = (chosen + remaining) if paths else []
        img_idx = 0

        def make_label_local(p: Optional[str]) -> QLabel:
            if not p:
                lbl = QLabel("")
                lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
                return lbl
            pm = QPixmap(str(p))
            if pm.isNull():
                lbl = QLabel(f"Missing: {os.path.basename(str(p))}")
                lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
                return lbl
            return ResizeImageLabel(pm)

        # For each macro slot, embed a small sub-grid (2x2 or 3x2 depending on span)
        rows, cols = self._canvas_rows_cols()
        spacing = max(4, min(10, self.grid.spacing()))
        for sl in scaled_slots:
            r, c, rh, cw = sl["row"], sl["col"], sl["height"], sl["width"]

            # Container frame to hold sub-grid; margins small to keep tight look
            container = QFrame()
            sub = QGridLayout()
            sub.setContentsMargins(spacing // 2, spacing // 2, spacing // 2, spacing // 2)
            sub.setSpacing(spacing // 2)
            container.setLayout(sub)

            # Decide sub-grid density based on macro span
            if rh * cw >= 6:
                sub_rows, sub_cols = 2, 3
            elif rh * cw >= 4:
                sub_rows, sub_cols = 2, 2
            elif rh * cw >= 3:
                sub_rows, sub_cols = 1, 3
            else:
                sub_rows, sub_cols = 1, max(1, cw)

            # Fill sub-grid with images
            for sr in range(sub_rows):
                for sc in range(sub_cols):
                    path = ordered_paths[img_idx % len(ordered_paths)] if ordered_paths else None
                    img_idx += 1 if ordered_paths else 0
                    sub.addWidget(make_label_local(path), sr, sc)
                sub.setRowStretch(sr, 1)
            for sc in range(sub_cols):
                sub.setColumnStretch(sc, 1)

            self.grid.addWidget(container, r, c, rh, cw)

        # Stretch outer grid
        for r in range(rows):
            self.grid.setRowStretch(r, 1)
        for c in range(cols):
            self.grid.setColumnStretch(c, 1)
