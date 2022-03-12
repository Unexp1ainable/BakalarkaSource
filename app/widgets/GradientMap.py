import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from src.opencv_qt_compat import *


class GradientMap(QWidget):
    MAP_SIZE = 100

    def __init__(self):
        super().__init__()
        layout = QGridLayout()
        self.map0Label = QLabel("No image")
        layout.addWidget(self.map0Label, 0, 0)
        self.map1Label = QLabel("No image")
        layout.addWidget(self.map1Label, 0, 1)
        self.map2Label = QLabel("No image")
        layout.addWidget(self.map2Label, 1, 1)
        self.map3Label = QLabel("No image")
        layout.addWidget(self.map3Label, 1, 0)

        self.mapOverviewLabel = QLabel("No image")
        layout.addWidget(self.mapOverviewLabel, 2, 0, 2, 2, Qt.AlignCenter)

        layout.setAlignment(Qt.AlignCenter)
        self.setLayout(layout)

    def setMap(self, path: string):
        self.map0 = cv.imread(path, cv.IMREAD_GRAYSCALE)
        self.map1 = cv.rotate(self.map0, cv.ROTATE_90_CLOCKWISE)
        self.map2 = cv.rotate(self.map1, cv.ROTATE_90_CLOCKWISE)
        self.map3 = cv.rotate(self.map2, cv.ROTATE_90_CLOCKWISE)

        mapAndLabelList = ((self.map0, self.map0Label), (self.map1, self.map1Label),
                           (self.map2, self.map2Label), (self.map3, self.map3Label))
        self.summed = np.zeros(self.map0.shape, np.int64)

        for map, label in mapAndLabelList:
            pixmap = mat2PixGray(cv.resize(map, (self.MAP_SIZE, self.MAP_SIZE), interpolation=cv.INTER_AREA))
            label.setPixmap(pixmap)
            self.summed += map

        self.summed = (self.summed // 4).astype(np.uint8)
        pixmap = mat2PixGray(cv.resize(self.summed, (self.MAP_SIZE, self.MAP_SIZE), interpolation=cv.INTER_AREA))
        self.mapOverviewLabel.setPixmap(pixmap.scaled(self.MAP_SIZE*2, self.MAP_SIZE*2, Qt.KeepAspectRatio))
