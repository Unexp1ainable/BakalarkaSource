from PySide6.QtGui import QPixmap, QResizeEvent
from PySide6.QtWidgets import QLabel, QSizePolicy, QFrame
from PySide6.QtCore import Qt, Signal
import PySide6


class ImageLabel(QLabel):
    dclicked = Signal(float, float)

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

    def mouseDoubleClickEvent(self, event: PySide6.QtGui.QMouseEvent) -> None:
        """Emit signal on double click. The signal has two parameters, each of them is in the range <0,1>,
           because image can be resized.

        Args:
            event (PySide6.QtGui.QMouseEvent): Mouse event
        """
        if event.buttons() & Qt.LeftButton:
            x = event.x() - ((self.width() - self.pixmap().width()) // 2)
            y = event.y() - ((self.height() - self.pixmap().height()) // 2)
            if (x < 0 or x > self.pixmap().width() or y < 0 or y > self.pixmap().height()):
                return
            self.dclicked.emit(x/self.pixmap().width(), y/self.pixmap().height())
