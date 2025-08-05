import os
import random
from PySide6.QtWidgets import QMainWindow, QScrollArea, QVBoxLayout, QWidget
from PySide6.QtCore import Qt, QSize
from bento_view import BentoView
from image_widget import ImageWidget

class GalleryWindow(QMainWindow):
    """
    The main window of the Aetherwave application.
    """
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Aetherwave Gallery")
        self.resize(1000, 800)  # Set a reasonable starting size

        # Load images from directory
        image_dir = "assets/images"
        try:
            image_files = [f for f in os.listdir(image_dir) if f.lower().endswith((".png", ".jpg", ".jpeg", ".gif", ".bmp"))]
            # Shuffle images for more visual interest
            random.shuffle(image_files)
        except FileNotFoundError:
            print(f"Warning: Directory not found: {image_dir}")
            image_files = []

        # Create image widgets
        image_widgets = [ImageWidget(os.path.join(image_dir, f)) for f in image_files]

        # Create container widget and scroll area
        container = QWidget()
        v_layout = QVBoxLayout(container)
        v_layout.setContentsMargins(0, 0, 0, 0)
        v_layout.setSpacing(0)

        # Create bento view with appropriate grid size
        columns = 5  # Use 5 columns for a nice bento grid
        cell_size = 180  # Slightly smaller cells for better visibility
        bento = BentoView(image_widgets, columns=columns, cell_size=cell_size)

        # Add bento to a scroll area
        scroll = QScrollArea()
        scroll.setWidget(bento)
        scroll.setWidgetResizable(False)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)

        # Add scroll area to layout
        v_layout.addWidget(scroll)

        # Set the container as central widget
        self.setCentralWidget(container)
