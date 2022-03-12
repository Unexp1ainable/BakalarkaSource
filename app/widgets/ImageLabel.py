from PySide6.QtGui import QPixmap, QResizeEvent
from PySide6.QtWidgets import QLabel, QSizePolicy, QFrame
from PySide6.QtCore import Qt


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

    def setPixmap(self, pm: QPixmap, h, w) -> None:
        self.pixmapOrig = pm
        self.setFrameStyle(QFrame.NoFrame)
        super(ImageLabel, self).setPixmap(pm.scaled(h, w, Qt.KeepAspectRatio))

    def resizeEvent(self, a0: QResizeEvent) -> None:
        if self.pixmapOrig:
            size = a0.size()
            super().setPixmap(self.pixmapOrig.scaled(size, Qt.KeepAspectRatio))
