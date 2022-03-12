import string
import cv2 as cv
import numpy as np
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPaintEvent, QPixmap, QTransform, QImage
from PySide6.QtCore import Slot, Qt
from PySide6 import QtCore
from widgets.ImageLabel import ImageLabel


class ImageLoader(QWidget):
    def __init__(self):
        super().__init__()
        layout = QGridLayout()

        self.img

        self.setLayout(layout)
