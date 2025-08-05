from PySide6.QtWidgets import QWidget, QGridLayout, QSizePolicy, QHBoxLayout, QVBoxLayout
from PySide6.QtCore import Qt

class BentoView(QWidget):
    """
    A widget that displays images in a bento box grid layout.
    """
    def __init__(self, image_widgets, columns=4, cell_size=200, parent=None):
        """
        Initializes the bento box view with a fixed grid layout.
        
        Args:
            image_widgets: List of ImageWidget instances.
            columns: Number of columns in the grid.
            cell_size: Size (pixels) for each cell (width and height).
            parent: Parent widget.
        """
        super().__init__(parent)
        
        # Create a container widget with fixed size policy
        container = QWidget()
        container.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)
        
        # Calculate rows needed
        num_images = len(image_widgets)
        rows = (num_images + columns - 1) // columns
        
        # Set container fixed size (account for 1px borders)
        container_width = columns * cell_size
        container_height = rows * cell_size
        container.setFixedSize(container_width, container_height)
        
        # Create grid layout for the container
        grid = QGridLayout(container)
        grid.setSpacing(0)
        grid.setContentsMargins(0, 0, 0, 0)
        
        # Add widgets to grid
        row = 0
        col = 0
        for widget in image_widgets:
            widget.setFixedSize(cell_size, cell_size)
            grid.addWidget(widget, row, col)
            col += 1
            if col >= columns:
                col = 0
                row += 1
                
        # Create vertical layout to hold the container
        v_layout = QVBoxLayout(self)
        v_layout.setSpacing(0)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.addWidget(container, 0, Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        v_layout.addStretch(1)
