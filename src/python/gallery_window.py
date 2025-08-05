import os
from PySide6.QtWidgets import QMainWindow
from bento_view import BentoView
from image_widget import ImageWidget

class GalleryWindow(QMainWindow):
    """
    The main window of the Aetherwave application.
    """
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Aetherwave Gallery")
        image_dir = "assets/images"
        image_files = [f for f in os.listdir(image_dir) if f.lower().endswith((".png", ".jpg", ".jpeg", ".gif", ".bmp"))]
        image_widgets = [ImageWidget(os.path.join(image_dir, f)) for f in image_files]
        columns = 4
        cell_size = 200
        bento = BentoView(image_widgets, columns=columns, cell_size=cell_size)
        from PySide6.QtWidgets import QScrollArea
        scroll = QScrollArea()
        scroll.setWidget(bento)
        scroll.setWidgetResizable(False)
        self.setCentralWidget(scroll)
