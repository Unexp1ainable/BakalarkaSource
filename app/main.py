import sys
from PySide6 import QtWidgets
from PySide6.QtCore import Qt
from widgets.MainWindow import MainWindow

if __name__ == "__main__":
    QtWidgets.QApplication.setHighDpiScaleFactorRoundingPolicy(Qt.HighDpiScaleFactorRoundingPolicy.Round)
    app = QtWidgets.QApplication([])

    window = MainWindow()
    window.resize(800, 450)
    window.show()

    sys.exit(app.exec())
