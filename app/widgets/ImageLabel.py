from PySide6.QtGui import QPixmap, QResizeEvent
from PySide6.QtWidgets import QLabel, QSizePolicy, QFrame
from PySide6.QtCore import Qt, Signal
import PySide6


class ImageLabel(QLabel):
    def __init__(self):
        super(ImageLabel, self).__init__()
        self.pixmapOrig = None
        self.setText("No image")
        self.setFrameStyle(QFrame.Box)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setMinimumSize(50, 50)
        self.setAlignment(Qt.AlignCenter)
        self.setMargin(0)

    def replacePixmap(self, pm: QPixmap) -> None:
        self.pixmapOrig = pm
        h = self.pixmap().height()
        w = self.pixmap().width()
        super().setPixmap(pm.scaled(w, h))

    def setPixmap(self, pm: QPixmap, h, w) -> None:
        self.pixmapOrig = pm
        self.setFrameStyle(QFrame.NoFrame)
        super().setPixmap(pm.scaled(w, h, Qt.KeepAspectRatio))

    def resizeEvent(self, a0: QResizeEvent) -> None:
        if self.pixmapOrig:
            size = a0.size()
            super().setPixmap(self.pixmapOrig.scaled(size, Qt.KeepAspectRatio))
