import string
from typing import List
import cv2 as cv
import numpy as np
from PySide6.QtSvgWidgets import QSvgWidget
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage, QMouseEvent
from PySide6.QtCore import Slot, Qt, Signal
from PySide6 import QtCore
from widgets.ImageLabel import ImageLabel
from src.opencv_qt_compat import *
from assets import bse_segments_assets


class SegmentManager(QWidget):
    swapped = Signal(int, int)

    def __init__(self):
        super().__init__()
        self.imgLabels: List[ImageLabel] = []
        self.imgs = []
        self.moving = None

        # self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        for i in range(4):
            self.imgLabels.append(ImageLabel())

        layout = QGridLayout()
        layout.addWidget(self.imgLabels[0], 0, 0, 2, 2)
        layout.addWidget(self.imgLabels[1], 0, 4, 2, 2)
        layout.addWidget(self.imgLabels[2], 2, 0, 2, 2)
        layout.addWidget(self.imgLabels[3], 2, 4, 2, 2)

        segmentImage = QSvgWidget(":/segments/bse_detector.svg")
        segmentImage.renderer().setAspectRatioMode(Qt.KeepAspectRatio)
        segmentImage.setMinimumSize(100, 100)
        segmentImage.setMaximumSize(150, 212)
        segmentImage.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        # segmentImage.setScaledContents(True)
        layout.addWidget(segmentImage, 1, 2, 2, 2)

        layout.setAlignment(segmentImage, Qt.AlignCenter)
        # layout.setColumnStretch(0, 300)
        # layout.setColumnStretch(1, 300)
        # layout.setColumnStretch(4, 300)
        # layout.setColumnStretch(5, 300)
        self.setLayout(layout)

    def loadImages(self, imgs):
        if (len(imgs) != 4):
            return

        self.imgs = imgs
        for i in range(4):
            self.imgLabels[i].setPixmap(mat2PixRGB(self.imgs[i]), 100, 100)

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        if event.buttons() & Qt.LeftButton:
            if self.moving == None:
                x = event.x()
                y = event.y()
                xwidg = self.width()
                ywidg = self.height()
                self.moving = x//(xwidg//2) + (y//(ywidg//2))*2

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
        if (not self.imgs):
            return
        tmp = self.imgs[first]
        self.imgs[first] = self.imgs[second]
        self.imgs[second] = tmp
        for i in range(4):
            self.imgLabels[i].replacePixmap(mat2PixRGB(self.imgs[i]))

        self.swapped.emit(first, second)
