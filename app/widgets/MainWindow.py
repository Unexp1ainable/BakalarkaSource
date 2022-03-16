from typing import List, Tuple
from PySide6.QtWidgets import *
from PySide6.QtGui import QAction, QPixmap, QTransform, QMouseEvent
from PySide6.QtCore import Slot, Qt
from widgets.GradientMap import GradientMap
from widgets.ImageManager import ImageManager
import cv2 as cv
import numpy as np
import traceback
import matplotlib.pyplot as plt
from scipy.spatial import KDTree


class MainWindow(QMainWindow):
    ALLOWED_DEVIATION = 20

    def __init__(self):
        super().__init__()

        self.maps = []
        self.sumMap = None
        self.imgs = []
        self.sumImg = None

        # init menubar
        menu = self.menuBar().addMenu("Load")
        self.loadImages = QAction("Load BSE images")
        self.loadMaps = QAction("Load gradient maps")
        menu.addAction(self.loadImages)
        menu.addAction(self.loadMaps)

        # init components
        self.map = GradientMap()
        self.map.setSizePolicy(QSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding))

        self.imageManager = ImageManager()
        self.imageManager.setSizePolicy(QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding))

        # create new layout of the main window
        layout = QHBoxLayout()

        w = QWidget()
        mapLayout = QVBoxLayout()
        mapLayout.addWidget(self.map)
        # mapLayout.addStretch(0)
        w.setLayout(mapLayout)

        splitter = QSplitter()
        splitter.addWidget(self.imageManager)
        splitter.addWidget(w)
        sizes = splitter.sizes()
        sizes[0] = 500
        sizes[1] = 100
        splitter.setSizes(sizes)

        layout.addWidget(splitter)

        # set layout of the main window
        mainWidget = QWidget()
        mainWidget.setLayout(layout)
        self.setCentralWidget(mainWidget)

        # connect signals
        self.loadMaps.triggered.connect(self.loadGradientMaps)
        self.loadImages.triggered.connect(self.loadBSEImages)
        self.imageManager.selector.dclicked.connect(self.onSelection)

        try:
            self.loadGradientMaps(["C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/sim.png"])
            filenames = ["../data/cinove_koule/25_mikro/cropped/25_mikro_3.tif",
                         "../data/cinove_koule/25_mikro/cropped/25_mikro_4.tif",
                         "../data/cinove_koule/25_mikro/cropped/25_mikro_5.tif",
                         "../data/cinove_koule/25_mikro/cropped/25_mikro_6.tif",
                         ]
            self.loadBSEImages(filenames)
        except BaseException as e:
            print(traceback.format_exc())

    @Slot()
    def loadGradientMaps(self, filename=None):
        if not filename:
            filename = QFileDialog.getOpenFileName()
        if (filename[0]):
            self.maps = []
            self.maps.append(cv.imread(filename[0]))
            for i in range(1, 4):
                self.maps.append(cv.rotate(self.maps[i-1], cv.ROTATE_90_CLOCKWISE))

            self.sumMap = np.zeros(self.maps[0].shape, np.int64)
            self.grayMaps = []

            for map in self.maps:
                self.sumMap += map
                self.grayMaps.append(cv.cvtColor(map, cv.COLOR_BGR2GRAY))

            self.sumMap = (self.sumMap // 4).astype(np.uint8)
            self.map.setMap(self.maps, self.sumMap)

    @Slot()
    def loadBSEImages(self, paths=None):
        if not paths:
            paths, _ = QFileDialog.getOpenFileNames()

        if (len(paths) != 4):
            return

        self.imgs = []
        for i in range(4):
            img = cv.imread(paths[i])
            if img is None:
                raise Exception("Failed to read images " + paths[i])
            self.imgs.append(img)

        self.sumImg = np.zeros(self.imgs[0].shape, np.uint16)
        for img in self.imgs:
            self.sumImg += img
        self.sumImg //= 4
        self.sumImg = self.sumImg.astype(np.uint8)

        self.imageManager.loadImages(self.imgs, self.sumImg)

    @Slot(float, float)
    def onSelection(self, x, y):
        x = int(self.imgs[0].shape[1] * x)
        y = int(self.imgs[0].shape[0] * y)
        self.map.color(self.imgs[0][y][x][0], self.imgs[1][y][x][0], self.imgs[2][y][x][0], self.imgs[3][y][x][0])

        mask0 = self.grayMaps[0] == self.imgs[0][y][x][0]
        mask1 = self.grayMaps[1] == self.imgs[1][y][x][0]
        mask2 = self.grayMaps[2] == self.imgs[2][y][x][0]
        mask3 = self.grayMaps[3] == self.imgs[3][y][x][0]
        masks = [mask0, mask1, mask2, mask3]

        mask01 = np.logical_and(mask0, mask1)
        mask02 = np.logical_and(mask0, mask2)
        mask03 = np.logical_and(mask0, mask3)
        mask12 = np.logical_and(mask1, mask2)
        mask13 = np.logical_and(mask1, mask3)
        mask23 = np.logical_and(mask2, mask3)
        andMasks: List[np.ndarray] = [(mask01, (0, 1)), (mask02, (0, 2)), (mask03, (0, 3)),
                                      (mask12, (1, 2)), (mask13, (1, 3)), (mask23, (2, 3))]

        finalPoints = []
        tentativePoints = []
        for mask, mapIndices in andMasks:
            # find furthest points
            width = mask.shape[1]
            height = mask.shape[0]
            up, down, right, left = (0, 0, 0, 0)
            # prepare normalization lambda
            def norm(pt): return np.array((pt[0]/height, pt[1]/width))

            # find boundaries
            for i in range(height):
                if np.any(mask[i]):
                    up = i
                    break

            for i in range(height-1, -1, -1):
                if np.any(mask[i]):
                    down = i
                    break
            tmask = mask.T
            for i in range(width):
                if np.any(tmask[i]):
                    left = i
                    break

            for i in range(width-1, -1, -1):
                if np.any(tmask[i]):
                    right = i
                    break

            # empty mask, find closest pair of points
            if up == down and right == left:
                points = self.findClosestPair(np.argwhere(masks[mapIndices[0]]), np.argwhere(masks[mapIndices[1]]))
                point = np.add(points[0], points[1])//2
                finalPoints.append(np.array(point))
                self.map.point(norm(point))
                continue

            # find furthest points
            point1 = None
            point2 = None
            if down-up > right-left:
                tmp = np.nonzero(mask[up])
                point1 = np.array((up, tmp[0][0]))
                tmp = np.nonzero(mask[down])
                point2 = np.array((down, tmp[0][-1]))
            else:
                tmp = np.nonzero(tmask[left])
                point1 = np.array((tmp[0][0], left))
                tmp = np.nonzero(tmask[right])
                point2 = np.array((tmp[0][-1], right))

            tentativePoints.append((point1, point2))
            # plot them
            self.map.point(norm(point1))
            self.map.point(norm(point2))

        if tentativePoints:
            avgPt = self.averageFinalTentative(finalPoints, tentativePoints)
            for pts in tentativePoints:
                def dist(pts): return np.sqrt(((pts[0]-pts[1])**2).sum())
                d1 = dist((pts[0], avgPt))
                d2 = dist((pts[1], avgPt))
                if abs(d1-d2) < MainWindow.ALLOWED_DEVIATION:
                    finalPoints.append(np.sum(pts, axis=0) / 2)
                elif d1 < d2:
                    finalPoints.append(pts[0])
                else:
                    finalPoints.append(pts[1])

        finalPt: np.ndarray = np.sum(finalPoints, axis=0) / 6
        self.map.point(norm(finalPt), Qt.white)

    def averageFinalTentative(self, final: List[Tuple[int, int]],
                              tentative: List[Tuple[Tuple[int, int],
                                                    Tuple[int, int]]]) -> Tuple[int, int]:
        avgPoint = np.zeros((1, 2))
        n = 0
        for pt in final:
            avgPoint += pt
            n += 1

        for pt in tentative:
            avgPoint += pt[0]
            avgPoint += pt[1]
            n += 2
        return avgPoint/n

    def findClosestPair(self, points1, points2):
        tree = KDTree(points1)
        a = tree.query(points2, 1)
        i = np.argmin(a[0])
        return (points2[i], points1[a[1][i]])
