from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPixmap, QTransform
from PySide6.QtCore import Slot, Qt
from widgets.GradientMap import GradientMap
from widgets.ImageLoader import ImageLoader


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        # init menubar
        menu = self.menuBar().addMenu("Load")
        self.loadImages = QAction("Load BSE images")
        self.loadMaps = QAction("Load gradient maps")
        menu.addAction(self.loadImages)
        menu.addAction(self.loadMaps)

        # init components
        self.map = GradientMap()
        self.imageLoader = ImageLoader()

        # create new layout of the main window
        layout = QGridLayout()
        layout.addWidget(self.map, 0, 1)

        # set layout of the main window
        mainWidget = QWidget()
        mainWidget.setLayout(layout)
        self.setCentralWidget(mainWidget)

        # connect signals
        self.loadMaps.triggered.connect(self.loadGradientMaps)
        self.loadImages.triggered.connect(self.loadBSEImages)

        try:
            self.map.setMap("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/sim.png")
        except:
            pass

    @Slot()
    def loadGradientMaps(self):
        filename = QFileDialog.getOpenFileName()
        if (filename[0]):
            self.map.setMap(filename[0])

    @Slot()
    def loadBSEImages(self):
        filenames, _ = QFileDialog.getOpenFileNames()
        if filenames:
            print(filenames)
