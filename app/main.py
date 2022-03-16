import sys
from PySide6 import QtWidgets
from PySide6.QtCore import Qt, QFile
from widgets.MainWindow import MainWindow

if __name__ == "__main__":
    QtWidgets.QApplication.setHighDpiScaleFactorRoundingPolicy(Qt.HighDpiScaleFactorRoundingPolicy.Round)
    app = QtWidgets.QApplication([])

    # load style
    try:
        with open("style.qss") as file:
            lines = file.read()
            app.setStyleSheet(lines)
    except:
        pass

    window = MainWindow()
    window.resize(800, 800)
    window.show()

    sys.exit(app.exec())
