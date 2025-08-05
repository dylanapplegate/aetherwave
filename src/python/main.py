import sys
from PySide6.QtWidgets import QApplication
from gallery_window import GalleryWindow

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = GalleryWindow()
    window.show()
    sys.exit(app.exec_())
