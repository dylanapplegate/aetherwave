import os
from PySide6.QtWidgets import QWidget
from PySide6.QtGui import QPainter, QPixmap
from PySide6.QtCore import Qt, QSize

class ImageWidget(QWidget):
    """
    A custom widget to display a QPixmap, handling scaling and aspect ratio correctly.
    """
    def __init__(self, image_path: str, parent=None):
        """
        Initializes the widget.
        Args:
            image_path: The absolute path to the image file.
        """
        super().__init__(parent)
        self.pixmap = QPixmap(image_path)
        self.setMinimumSize(1, 1)
        from PySide6.QtWidgets import QSizePolicy
        self.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)

    def setPixmap(self, pixmap: QPixmap):
        """
        Sets the pixmap for the widget.
        Args:
            pixmap: The QPixmap object to display.
        """
        self.pixmap = pixmap
        self.update()  # Trigger a repaint

    def paintEvent(self, event):
        """
        Handles the painting of the widget, scaling the pixmap to fill the cell (cropping if needed).
        """
        painter = QPainter(self)
        if self.pixmap.isNull():
            from PySide6.QtGui import QColor
            painter.setPen(QColor(0, 0, 0))
            from PySide6.QtCore import Qt as QtCoreQt
            painter.drawText(self.rect(), QtCoreQt.AlignmentFlag.AlignCenter, "Image not found")
            return

        # Scale pixmap to fill widget size, cropping if necessary
        from PySide6.QtCore import Qt as QtCoreQt
        scaled_pixmap = self.pixmap.scaled(self.size(), QtCoreQt.AspectRatioMode.KeepAspectRatioByExpanding, QtCoreQt.TransformationMode.SmoothTransformation)
        painter.drawPixmap(0, 0, scaled_pixmap)

    def sizeHint(self) -> QSize:
        """
        Provides a size hint based on the pixmap's original size.
        """
        if self.pixmap.isNull():
            return QSize(100, 100)
        return self.pixmap.size()
