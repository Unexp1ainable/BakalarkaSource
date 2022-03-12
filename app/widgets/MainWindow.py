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
        self.map.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding))

        self.imageLoader = ImageLoader()
        self.imageLoader.setSizePolicy(QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding))

        # create new layout of the main window
        layout = QGridLayout()
        layout.setColumnStretch(0, 1)
        layout.addWidget(self.imageLoader, 0, 0)
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
            filenames = ["data/cinove_koule/25_mikro/cropped/25_mikro_3.tif",
                         "data/cinove_koule/25_mikro/cropped/25_mikro_4.tif",
                         "data/cinove_koule/25_mikro/cropped/25_mikro_5.tif",
                         "data/cinove_koule/25_mikro/cropped/25_mikro_6.tif",
                         ]
            self.imageLoader.loadImages(filenames)
        except:
            pass

    @ Slot()
    def loadGradientMaps(self):
        filename = QFileDialog.getOpenFileName()
        if (filename[0]):
            self.map.setMap(filename[0])

    @ Slot()
    def loadBSEImages(self):
        filenames, _ = QFileDialog.getOpenFileNames()
        if filenames:
            self.imageLoader.loadImages(filenames)
