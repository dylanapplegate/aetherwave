from PySide6.QtWidgets import QWidget, QGridLayout, QSizePolicy

class BentoView(QWidget):
    """
    A widget that displays images in a grid layout.
    """
    def __init__(self, image_widgets, columns=4, cell_size=200, parent=None):
        super().__init__(parent)
        layout = QGridLayout()
        layout.setSpacing(0)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)
        self.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        self.setFixedSize(columns * cell_size, ((len(image_widgets) + columns - 1) // columns) * cell_size)
        row = 0
        col = 0
        for widget in image_widgets:
            widget.setFixedSize(cell_size, cell_size)
            layout.addWidget(widget, row, col)
            col += 1
            if col >= columns:
                col = 0
                row += 1

    def add_images_grid(self, image_widgets, columns: int = 4, cell_size: int = 200):
        """
        Adds image widgets to the grid in a bento box style.
        Args:
            image_widgets: List of ImageWidget instances.
            columns: Number of columns in the grid.
            cell_size: Size (pixels) for each cell (width and height).
        """
        row = 0
        col = 0
        for widget in image_widgets:
            widget.setFixedSize(cell_size, cell_size)
            self._layout.addWidget(widget, row, col)
            col += 1
            if col >= columns:
                col = 0
                row += 1
