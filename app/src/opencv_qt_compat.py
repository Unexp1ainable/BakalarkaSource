import numpy as np
from PySide6.QtGui import QPixmap, QImage


def mat2PixGray(img: np.ndarray) -> QPixmap:
    return QPixmap.fromImage(QImage(img.data, img.shape[1], img.shape[0], QImage.Format_Grayscale8))


def mat2PixRGB(img: np.ndarray) -> QPixmap:
    return QPixmap.fromImage(QImage(img.data, img.shape[1], img.shape[0], QImage.Format_BGR888))
