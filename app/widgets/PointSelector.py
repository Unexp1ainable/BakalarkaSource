import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage, QMouseEvent
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from widgets.ImageLabel import ImageLabel
from widgets.SegmentManager import SegmentManager
from src.opencv_qt_compat import *


class PointSelector(QWidget):
    def __init__(self):
        super().__init__()

        self.selector = ImageLabel()
        layout = QVBoxLayout()
        layout.addWidget(self.selector)
        self.setLayout(layout)

    def loadImage(self, sumImg):
        self.summed = sumImg
        self.selector.setPixmap(mat2PixRGB(self.summed), 200, 200)
