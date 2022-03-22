from copy import deepcopy
import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage, QPainter, QColor, QPen, QBrush
from PySide6.QtCore import Slot, Qt, QPoint
from PySide6 import QtCore
from src.opencv_qt_compat import *
from widgets.ImageLabel import *


class ReflectanceMap(QWidget):
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
        innerLayout.addWidget(self.map2Label, 1, 0)
        self.map3Label = ImageLabel()
        innerLayout.addWidget(self.map3Label, 1, 1)

        self.labels = (self.map0Label, self.map1Label, self.map2Label, self.map3Label,)

        layout = QVBoxLayout()
        self.mapOverviewLabel = ImageLabel()
        pqLabel = QLabel("p,q:")
        self.gaussCoords = QLabel()
        hL = QHBoxLayout()
        hL.addWidget(pqLabel)
        hL.addWidget(self.gaussCoords)

        layout.setAlignment(Qt.AlignHCenter)
        layout.addWidget(w)
        layout.addWidget(self.mapOverviewLabel)

        layout.addLayout(hL)
        self.setLayout(layout)

        self.setMinimumSize(150, 300)

    def setMap(self, imgs, summed):
        self.imgs = imgs
        self.sumImg = summed
        self.maps = []
        for img in imgs:
            self.maps.append(cv.cvtColor(img, cv.COLOR_BGR2GRAY))

        mapAndLabelList = ((imgs[0], self.map0Label), (imgs[1], self.map1Label),
                           (imgs[2], self.map2Label), (imgs[3], self.map3Label))

        for map, label in mapAndLabelList:
            pixmap = mat2PixRGB(map)
            label.setPixmap(pixmap, self.MAP_SIZE, self.MAP_SIZE)

        pixmap = mat2PixRGB(summed)
        self.mapOverviewLabel.setPixmap(pixmap, self.MAP_SIZE*2, self.MAP_SIZE*2)

    def color(self, a, b, c, d):
        it = (a, b, c, d)
        colors = ((0, 0, 255), (0, 255, 0), (255, 0, 0), (0, 255, 255))
        sumcopy = deepcopy(self.sumImg)
        for i in range(4):
            img = deepcopy(self.imgs[i])
            mask = self.maps[i] == it[i]
            img[mask] = colors[i]
            sumcopy[mask] = colors[i]
            pixmap = mat2PixRGB(img)
            self.labels[i].replacePixmap(pixmap)
        self.mapOverviewLabel.replacePixmap(mat2PixRGB(sumcopy))

    def point(self, pt, color=Qt.magenta):
        pix = self.mapOverviewLabel.pixmap()
        painter = QPainter(pix)
        pen = QPen(color, 3)
        brush = QBrush(color)
        painter.setPen(pen)
        painter.setBrush(brush)
        painter.drawEllipse(QPoint(int(pt[1]*pix.width()), int(pt[0]*pix.height())), 3, 3)
        painter.end()
        self.mapOverviewLabel.replacePixmap(pix)

    def setPQ(self, p, q):
        self.gaussCoords.setText(str(p) + ", " + str(q))
