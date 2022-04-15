import numpy as np
import cv2 as cv
from ellipse_common import *
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

    def rasterize(self):
        return raster_ellipse(self.a, self.b, self.c, self.h, self.k)

    def draw(self, img):
        return draw_ellipse(img, self.a, self.b, self.c, self.h, self.k)

    def intersections(self, el2: "Superellipse"):
        if not(
                self.bBox[1][1] < el2.bBox[0][1] and el2.bBox[1][1] < self.bBox[0][1]) or not(
                self.bBox[1][0] > el2.bBox[0][0] and el2.bBox[1][0] > self.bBox[0][0]):
            # print("No intersection")
            return ((0, 0), (0, 0))
        roiUp = self.bBox[0][1] if self.bBox[0][1] < el2.bBox[0][1] else el2.bBox[0][1]
        roiDown = self.bBox[1][1] if self.bBox[1][1] > el2.bBox[1][1] else el2.bBox[1][1]
        roiLeft = self.bBox[0][0] if self.bBox[0][0] > el2.bBox[0][0] else el2.bBox[0][0]
        roiRight = self.bBox[1][0] if self.bBox[1][0] < el2.bBox[1][0] else el2.bBox[1][0]
        # return ((roiLeft, roiUp), (roiRight, roiDown))
        lside1 = 0
        lside2 = 0
        sside1 = 0
        sside2 = 0
        roots = rootsX
        mode = X
        # select longer side
        if roiDown-roiUp > roiLeft-roiRight:
            # y known, x calculated
            lside1 = roiDown
            lside2 = roiUp
            sside1 = roiLeft
            sside2 = roiRight
            roots = rootsX
            mode = X
        else:
            # x known, y calculated
            lside1 = roiLeft
            lside2 = roiRight
            sside1 = roiDown
            sside2 = roiUp
            roots = rootsY
            mode = Y

        results11 = []
        results12 = []
        results21 = []
        results22 = []

        for i in range(lside1, lside2):
            n1, r11, r12 = roots(self.a, self.b, self.c, self.h, self.k, i)
            n2, r21, r22 = roots(el2.a, el2.b, el2.c, el2.h, el2.k, i)

            if n1 == 0 or n2 == 0:
                results11.append(None)
                results21.append(None)
                results12.append(None)
                results22.append(None)

            if sside1 < r11 < sside2:
                results11.append(r11)
            else:
                results11.append(None)

            if sside1 < r21 < sside2:
                results21.append(r21)
            else:
                results21.append(None)

            if n1 == 2 and sside1 < r12 < sside2:
                results12.append(r12)
            else:
                results12.append(None)

            if n2 == 2 and sside1 < r22 < sside2:
                results22.append(r22)
            else:
                results22.append(None)

        diff1 = []
        diff2 = []
        diff3 = []
        diff4 = []
        i = 0
        for _ in range(lside1, lside2):
            r11 = results11[i]
            r12 = results12[i]
            r21 = results21[i]
            r22 = results22[i]

            if r11 is not None:
                if r21 is not None:
                    diff1.append(abs(r11-r21))
                else:
                    diff1.append(BIG)

                if r22 is not None:
                    diff2.append(abs(r11-r22))
                else:
                    diff2.append(BIG)
            else:
                diff1.append(BIG)
                diff2.append(BIG)

            if r12 is not None:
                if r21 is not None:
                    diff3.append(abs(r12-r21))
                else:
                    diff3.append(BIG)

                if r22 is not None:
                    diff4.append(abs(r12-r22))
                else:
                    diff4.append(BIG)
            else:
                diff3.append(BIG)
                diff4.append(BIG)
            i += 1

        # plt.plot(diff1, label="1")
        # plt.plot(diff2, label="2")
        # plt.plot(diff3, label="3")
        # plt.plot(diff4, label="4")
        # plt.show()
        diffs = (diff1, diff2, diff3, diff4)
        results = (results11, results11, results21, results22)
        mins = []
        i = 0
        for diff in diffs:
            if len(diff) > 0:
                mins.append((i, min(diff)))
            i += 1

        mins.sort(key=lambda x: x[1])
        res1 = np.argmin(diffs[mins[0][0]])
        res2 = np.argmin(diffs[mins[1][0]])

        if mode == X:
            return (round(results[mins[0][0]][res1]), res1+lside1), (round(results[mins[1][0]][res2]), res2+lside1)
        else:
            return (res1+lside1, round(results[mins[0][0]][res1])), (res2+lside1, round(results[mins[1][0]][res2]))


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
    while True:
        img = np.zeros((H, W), np.uint8)
        el1.draw(img)
        el2.draw(img)
        pt1, pt2 = el1.intersections(el2)

        pt1 = (pt1[0]+W//2, -pt1[1])
        pt2 = (pt2[0]+W//2, -pt2[1])
        pt1 = clamp(pt1)
        pt2 = clamp(pt2)
        # img = cv.rectangle(img, pt1, pt2, 100, 1)
        img = cv.circle(img, pt1, 5, 255)
        img = cv.circle(img, pt2, 5, 255)
        cv.imshow("img", img)
        key = cv.waitKey(10)
        if key == 27:  # esc
            break
    cv.destroyAllWindows()
