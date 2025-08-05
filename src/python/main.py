import sys
import traceback
from PySide6.QtWidgets import QApplication, QMessageBox
from gallery_window import GalleryWindow

def exception_hook(exc_type, exc_value, exc_traceback):
    """Handle uncaught exceptions to show them in a dialog box"""
    error_msg = ''.join(traceback.format_exception(exc_type, exc_value, exc_traceback))
    print(error_msg)  # Print to console
    
    # Create a message box to show the error
    msg_box = QMessageBox()
    msg_box.setIcon(QMessageBox.Icon.Critical)
    msg_box.setText("An unexpected error occurred")
    msg_box.setInformativeText(str(exc_value))
    msg_box.setDetailedText(error_msg)
    msg_box.setWindowTitle("Error")
    msg_box.exec_()

if __name__ == "__main__":
    # Set up exception handling
    sys.excepthook = exception_hook
    
    # Create and run application
    app = QApplication(sys.argv)
    try:
        window = GalleryWindow()
        window.show()
        sys.exit(app.exec_())
    except Exception as e:
        exception_hook(type(e), e, e.__traceback__)
