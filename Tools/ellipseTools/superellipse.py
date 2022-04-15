from matplotlib.transforms import Bbox
import numpy as np
import cv2 as cv
from ellipse_common import *
from scipy.spatial import KDTree
import matplotlib.pyplot as plt

BIG = 99999999999999

X = 0
Y = 1


class Superellipse():
    def __init__(self, a, b, c, h, k) -> None:
        self.a = a
        self.b = b
        self.c = c
        self.h = h
        self.k = k
        self.calculateBoundingBox()

    def calculateBoundingBox(self):
        self.bBox = ((-self.h-self.a, -self.k), (-self.h+self.a, -self.k-2*self.b))

    def setH(self, h):
        self.h = h
        self.calculateBoundingBox()

    def setK(self, k):
        self.k = k
        self.calculateBoundingBox()

    def autoroots(self, x, y):
        dx = x-self.bBox[0][0] if x-self.bBox[0][0] < self.bBox[1][0] - x else self.bBox[1][0] - x
        dy = y-self.bBox[1][1] if y-self.bBox[1][1] < self.bBox[0][1] - y else self.bBox[0][1] - y

        if dx < dy:
            n, x1, x2 = rootsX(self.a, self.b, self.c, self.h, self.k, y)
            nx = 0
            if n == 0:
                return None, None

            if n == 1:
                nx = x1
            else:
                if abs(x1-x) < abs(x2-x):
                    nx = x1
                else:
                    nx = x2

            return nx, y
        else:
            n, y1, y2 = rootsY(self.a, self.b, self.c, self.h, self.k, x)
            ny = 0
            if n == 0:
                return None, None

            if n == 1:
                ny = y1
            else:
                if abs(y1-y) < abs(y2-y):
                    ny = y1
                else:
                    ny = y2

            return x, ny

    def rasterize(self):
        return raster_ellipse(self.a, self.b, self.c, self.h, self.k)

    def draw(self, img):
        return draw_ellipse(img, self.a, self.b, self.c, self.h, self.k)

    def borderPointsGenerator(self):
        """Generates points starting from the top middle to the right around whole ellipse
        """
        x, y = (-self.h, -self.k)
        # 2nd top half
        # print("top")
        while (True):
            yield x, y
            x += 1
            n, y1, y2 = rootsY(self.a, self.b, self.c, self.h, self.k, x)
            ny = 0
            if n == 0:
                x -= 1
                break
            elif n == 1:
                ny = y1
            else:
                ny = y1 if abs(y1-y) < abs(y2-y) else y2

            if abs(y-ny) > 1:
                x -= 1
                break
            y = ny

        # right side
        # print("right")
        while (True):
            y -= 1
            n, x1, x2 = rootsX(self.a, self.b, self.c, self.h, self.k, y)
            nx = 0
            if n == 0:
                y += 1
                break
            elif n == 1:
                nx = x1
            else:
                nx = x1 if abs(x1-x) < abs(x2-x) else x2

            if abs(x-nx) > 1:
                y += 1
                break
            x = nx
            yield x, y

        # bottom side
        # print("bottom")
        while (True):
            x -= 1
            n, y1, y2 = rootsY(self.a, self.b, self.c, self.h, self.k, x)
            ny = 0
            if n == 0:
                x += 1
                break
            elif n == 1:
                ny = y1
            else:
                ny = y1 if abs(y1-y) < abs(y2-y) else y2

            if abs(y-ny) > 1:
                x += 1
                break
            y = ny
            yield x, y

        # left side
        # print("left")
        while (True):
            y += 1
            n, x1, x2 = rootsX(self.a, self.b, self.c, self.h, self.k, y)
            nx = 0
            if n == 0:
                y -= 1
                break
            elif n == 1:
                nx = x1
            else:
                nx = x1 if abs(x1-x) < abs(x2-x) else x2

            if abs(x-nx) > 1:
                y -= 1
                break
            x = nx
            yield x, y

        # 1st top half
        # print("top")
        while (x < -self.h):
            x += 1
            n, y1, y2 = rootsY(self.a, self.b, self.c, self.h, self.k, x)
            ny = 0
            if n == 0:
                x -= 1
                break
            elif n == 1:
                ny = y1
            else:
                ny = y1 if abs(y1-y) < abs(y2-y) else y2

            if abs(y-ny) > 1:
                x -= 1
                break
            y = ny
            yield x, y

    def findClosestPoints(self, el2: "Superellipse"):
        """Call this function if you are sure that there is no intersection
        """
        points1 = self.rasterize()
        points2 = el2.rasterize()
        tree = KDTree(points1)
        a = tree.query(points2, 1)
        i = np.argmin(a[0])

        pt1 = points1[a[1][i]]
        pt2 = points2[i]
        a = (pt1+pt2)/2
        return [a]

        return ((pt1[0] + pt2[0])/2, (pt1[1]+pt2[1])/2)

    def intersections(self, el2: "Superellipse"):
        lastDiff = BIG
        lastY = 0
        lastX = 0
        expectIntersection = False
        recovering = False
        res = []
        for x, y in self.borderPointsGenerator():
            xn, yn = el2.autoroots(x, y)
            if xn == None:
                continue
            diff = abs(y-yn) + abs(x-xn)
            if expectIntersection:
                if lastDiff < diff:
                    res.append((lastX, lastY))
                    expectIntersection = False
                    recovering = True
                    continue
                else:
                    lastDiff = diff
                    lastX = x
                    lastY = y
                    continue

            if diff < 1 and not recovering:
                expectIntersection = True
                lastDiff = diff
                lastX = x
                lastY = y

            if diff > 1:
                recovering = False

        return res

    def findPOI(self, el2: "Superellipse"):
        inters = self.intersections(el2)
        if len(inters) == 0:
            return self.findClosestPoints(el2)
        return inters


H = 500
W = 500

global px, py, el2
px = None
py = None
el2 = None


def mouseCallback(event, x, y, flags, data):
    global px, py
    if event == cv.EVENT_LBUTTONDOWN:
        px = x
        py = y

    elif event == cv.EVENT_LBUTTONUP:
        print(x, y)
        px = None

    elif event == cv.EVENT_MOUSEMOVE:
        if px is not None:
            el2.setH(-x + H//2)
            el2.setK(y)


def clamp(x):
    res1 = 0
    res2 = 0
    if x[0] > 10000:
        res1 = 0
    else:
        res1 = x[0]
    if x[1] > 1000:
        res2 = 0
    else:
        res2 = x[1]
    return res1, res2


if __name__ == "__main__":
    el1 = Superellipse(100, 50, 2.5, 10, 150)
    el2 = Superellipse(100, 100, 2.5, 0, 0)

    w = cv.namedWindow("img")
    cv.setMouseCallback("img", mouseCallback)

    # for pt1 in el1.borderPointsGenerator():
    #     img = np.zeros((H, W), np.uint8)
    #     el1.draw(img)
    #     pt1 = (round(pt1[0]+W//2), round(-pt1[1]))
    #     pt1 = clamp(pt1)
    #     # img = cv.rectangle(img, pt1, pt2, 100, 1)
    #     img = cv.circle(img, pt1, 5, 255)
    #     cv.imshow("img", img)
    #     key = cv.waitKey(10)
    #     if key == 27:  # esc
    #         break

    while True:
        img = np.zeros((H, W), np.uint8)
        el1.draw(img)
        el2.draw(img)
        pts = el1.findPOI(el2)
        # img = cv.rectangle(img, pt1, pt2, 100, 1)

        for pt in pts:
            pt = (round(pt[0]+W//2), round(-pt[1]))
            pt = clamp(pt)
            img = cv.circle(img, pt, 5, 255)
        cv.imshow("img", img)
        key = cv.waitKey(10)
        if key == 27:  # esc
            break
    cv.destroyAllWindows()
