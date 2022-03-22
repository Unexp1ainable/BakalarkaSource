import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage, QMouseEvent
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from widgets.ImageLabel import ImageLabel
from widgets.SegmentManager import SegmentManager
from widgets.PointSelector import PointSelector
from src.opencv_qt_compat import *


class ImageManager(QWidget):
    def __init__(self):
        super().__init__()
        self.imgLabels = []
        self.imgs = []
        self.moving = None

        self.segmentManager = SegmentManager()
        self.selector = ImageLabel()
        self.selector.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layout = QVBoxLayout()
        layout.addWidget(self.segmentManager, 1)
        layout.addWidget(self.selector, 2)
        self.setLayout(layout)

    def loadImages(self, imgs, sumImg):
        if (len(imgs) != 4):
            return

        self.imgs = imgs

        self.segmentManager.loadImages(self.imgs)

        self.summed = sumImg
        self.selector.setPixmap(mat2PixRGB(self.summed), 200, 200)
