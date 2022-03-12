import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage, QMouseEvent
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from widgets.ImageLabel import ImageLabel
from src.opencv_qt_compat import *


class ImageLoader(QWidget):
    def __init__(self):
        super().__init__()
        layout = QGridLayout()
        self.imgLabels = []
        self.imgs = []
        self.moving = None

        for i in range(4):
            self.imgLabels.append(ImageLabel())

        layout.addWidget(self.imgLabels[0], 0, 0)
        layout.addWidget(self.imgLabels[1], 0, 1)
        layout.addWidget(self.imgLabels[2], 1, 0)
        layout.addWidget(self.imgLabels[3], 1, 1)
        self.setLayout(layout)

    def loadImages(self, paths):
        if (len(paths) != 4):
            return

        self.imgs = []
        for i in range(4):
            self.imgs.append(cv.imread(paths[i]))
            self.imgLabels[i].setPixmap(mat2PixRGB(self.imgs[i]), 100, 100)

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        if event.buttons() & Qt.LeftButton:
            if self.moving == None:
                x = event.x()
                y = event.y()
                xwidg = self.width()
                ywidg = self.height()
                self.moving = x//(xwidg//2) + (y//(ywidg//2))*2
                print(self.moving)

    def mouseReleaseEvent(self, event: QMouseEvent) -> None:
        if event.button() == Qt.LeftButton:
            if self.moving != None:
                moving = self.moving
                self.moving = None
                x = event.x()
                y = event.y()
                xwidg = self.width()
                ywidg = self.height()
                target = x//(xwidg//2) + (y//(ywidg//2))*2
                if target < 0 or target == moving:
                    return
                self.swapImages(moving, target)

    def swapImages(self, first: int, second: int):
        tmp = self.imgs[first]
        self.imgs[first] = self.imgs[second]
        self.imgs[second] = tmp
        for i in range(4):
            self.imgLabels[i].replacePixmap(mat2PixRGB(self.imgs[i]))
