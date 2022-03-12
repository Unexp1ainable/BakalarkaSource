from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPixmap, QTransform
from PySide6.QtCore import Slot, Qt
import widgets.GradientMap as gm


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        menu = self.menuBar().addMenu("File")
        self.loadMaps = QAction("Load gradient maps")
        menu.addAction(self.loadMaps)

        layout = QGridLayout()
        self.maps = QWidget()

        l = QGridLayout()
        self.map = gm.GradientMap()
        l.addWidget(self.map, 0, 0)
        self.maps.setLayout(l)
        layout.addWidget(self.maps, 1, 1)

        mainWidget = QWidget()
        mainWidget.setLayout(layout)
        self.setCentralWidget(mainWidget)

        self.loadMaps.triggered.connect(self.loadGradientMaps)
        try:
            self.map.setMap("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/sim.png")
        except:
            pass

    @Slot()
    def loadGradientMaps(self):
        filename = QFileDialog.getOpenFileName()
        if (filename[0]):
            self.map.setMap(filename[0])
