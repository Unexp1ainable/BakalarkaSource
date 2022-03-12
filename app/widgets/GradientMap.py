import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from src.opencv_qt_compat import *
from widgets.ImageLabel import *


class GradientMap(QWidget):
    MAP_SIZE = 100

    def __init__(self):
        super().__init__()
        innerLayout = QGridLayout()
        innerLayout.setContentsMargins(0, 0, 0, 0)
        w = QWidget()
        w.setLayout(innerLayout)
        self.map0Label = ImageLabel()
        innerLayout.addWidget(self.map0Label, 0, 0)
        self.map1Label = ImageLabel()
        innerLayout.addWidget(self.map1Label, 0, 1)
        self.map2Label = ImageLabel()
        innerLayout.addWidget(self.map2Label, 1, 1)
        self.map3Label = ImageLabel()
        innerLayout.addWidget(self.map3Label, 1, 0)

        layout = QVBoxLayout()
        self.mapOverviewLabel = ImageLabel()
        layout.setAlignment(Qt.AlignHCenter)
        layout.addWidget(w)
        layout.addWidget(self.mapOverviewLabel)
        self.setLayout(layout)

        self.setMinimumSize(150, 300)

    def setMap(self, path: string):
        self.map0 = cv.imread(path, cv.IMREAD_GRAYSCALE)
        self.map1 = cv.rotate(self.map0, cv.ROTATE_90_CLOCKWISE)
        self.map2 = cv.rotate(self.map1, cv.ROTATE_90_CLOCKWISE)
        self.map3 = cv.rotate(self.map2, cv.ROTATE_90_CLOCKWISE)

        mapAndLabelList = ((self.map0, self.map0Label), (self.map1, self.map1Label),
                           (self.map2, self.map2Label), (self.map3, self.map3Label))
        self.summed = np.zeros(self.map0.shape, np.int64)

        for map, label in mapAndLabelList:
            pixmap = mat2PixGray(map)
            label.setPixmap(pixmap, self.MAP_SIZE, self.MAP_SIZE)
            self.summed += map

        self.summed = (self.summed // 4).astype(np.uint8)
        pixmap = mat2PixGray(self.summed)
        self.mapOverviewLabel.setPixmap(pixmap, self.MAP_SIZE*2, self.MAP_SIZE*2)
