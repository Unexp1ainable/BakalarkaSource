from scipy.interpolate import PchipInterpolator
from math import isclose, log, sqrt
from multiprocessing.synchronize import Lock
from os import mkdir
from typing import List, Tuple
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from scipy import ndimage
from skimage.morphology import skeletonize
from numba import jit
from ellipse_common import *


UP = True
DOWN = False

STEP_A = 2
STEP_B = 2
STEP_C = 0.1
STEP_K = 0.5

DIR_A = None
DIR_B = None
DIR_C = None
DIR_K = None

ROUGH_ITERATIONS = 10
SMOOTH_ITERATIONS = 10
VISUALIZE = False


def show(a, b, c, k):
    dimg = img.copy()
    draw_ellipse(dimg, a, b, c, k)
    cv.imshow("tmp", dimg)
    cv.waitKey(1)


def fit_a(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:

    pts = [
        (0.0, 422.47265625),
        (7.5, 441.7734375),
        (15.0, 461.07421875),
        (22.5, 471.796875),
        (30.0, 484.6640625),
        (37.75, 491.09765625),
        (45.25, 503.96484375),
        (52.75, 510.3984375),
        (60.25, 512.54296875),
        (67.75, 512.54296875),
        (75.25, 514.6875),
        (82.75, 510.3984375),
        (90.25, 501.8203125),
        (98.0, 491.09765625),
        (105.5, 488.953125),
        (113.0, 476.0859375),
        (120.5, 465.36328125),
        (128.0, 456.78515625),
        (135.5, 439.62890625),
        (143.0, 431.05078125),
        (150.5, 418.18359375),
        (158.0, 396.73828125),
        (165.75, 360.28125),
        (173.25, 336.69140625),
        (180.75, 302.37890625),
        (188.25, 259.48828125),
        (195.75, 199.44140625),
        (203.25, 120.09375),
        (210.75, 0.0),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]


def fit_b(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> float:

    pts = [
        (0.0, 244.953125),
        (7.5, 242.9453125),
        (15.0, 240.9375),
        (22.5, 239.93359375),
        (30.0, 240.9375),
        (37.75, 241.94140625),
        (45.25, 242.9453125),
        (52.75, 243.94921875),
        (60.25, 245.95703125),
        (67.75, 245.95703125),
        (75.25, 243.94921875),
        (82.75, 242.9453125),
        (90.25, 239.93359375),
        (98.0, 237.92578125),
        (105.5, 236.921875),
        (113.0, 233.91015625),
        (120.5, 234.9140625),
        (128.0, 233.91015625),
        (135.5, 230.8984375),
        (143.0, 227.88671875),
        (150.5, 217.84765625),
        (158.0, 204.796875),
        (165.75, 183.71484375),
        (173.25, 159.62109375),
        (180.75, 136.53125),
        (188.25, 113.44140625),
        (195.75, 82.3203125),
        (203.25, 42.1640625),
        (210.75, 0.0),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]


# @jit(nopython=True)
def fit_c(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, h, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a, b, c + STEP_C, h, k, points, height, width)
    if lastRank < currRank:
        DIR_C = DOWN
    else:
        DIR_C = UP
        c += STEP_C

    while True:
        if DIR_C == UP:
            currRank = rank_ellipse(a, b, c + STEP_C, h, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                c += STEP_C
                if c > 7:
                    break
        else:
            currRank = rank_ellipse(a, b, c - STEP_C, h, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                if c - STEP_C <= 1.5:
                    break
                c -= STEP_C

    # if True:
    #     show(a, b, c, k)
    return c

    # return 2.13


def fit_k(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    pts = [
        (0.0, 0.0),
        (7.5, 0.0),
        (15.0, -1.775390625),
        (22.5, 81.66796875),
        (30.0, 152.68359375),
        (37.75, 184.640625),
        (45.25, 213.046875),
        (52.75, 236.126953125),
        (60.25, 259.20703125),
        (67.75, 284.0625),
        (75.25, 305.3671875),
        (82.75, 330.22265625),
        (90.25, 356.853515625),
        (98.0, 383.484375),
        (105.5, 408.33984375),
        (113.0, 438.521484375),
        (120.5, 465.15234375),
        (128.0, 488.232421875),
        (135.5, 523.740234375),
        (143.0, 548.595703125),
        (150.5, 577.001953125),
        (158.0, 603.6328125),
        (165.75, 642.69140625),
        (173.25, 674.6484375),
        (180.75, 717.2578125),
        (188.25, 759.8671875),
        (195.75, 807.802734375),
        (203.25, 866.390625),
        (210.75, 909.0),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]


def fit_ellipse(img: np.ndarray, value: int = None, name: str = "") -> Tuple[float, float, float, float, float]:
    if type(img) == tuple:  # multiprocessing arguments
        value = img[1]
        name = img[2]
        img = img[0]

    print("Started with value ", value)
    if value is not None:
        img = processing(img, value)

    bounds = get_bounds(img)
    if bounds[0] >= bounds[2] or bounds[1] >= bounds[3]:
        print("Error, no pixels of interest. " + str(value))
        return (0., 0., 0., 0., 0.)

    points = np.argwhere(img)
    height = img.shape[0]
    width = img.shape[1]

    a = round((bounds[2]-bounds[0])*0.5)
    b = a/2
    c = 2
    k = -bounds[1]

    a = fit_a(a, b, c, 0, k, points, height, width, value)
    b = fit_b(a, b, c, 0, k, points, height, width, value)
    k = fit_k(a, b, c, 0, k, points, height, width, value)
    c = fit_c(a, b, c, 0, k, points, height, width, value)

    print("convergence reached " + str(value))
    draw_ellipse(img, a, b, c, 0, k)

    outdir = "output/"
    if name:
        outdir = "output/" + name[:-4] + "/"

    cv.imwrite(outdir + "img" + str(value) + ".png", img)

    try:
        with open(outdir + "output.csv", "a") as file:
            file.write(str(value) + ";" + str(a) + ";" + str(b) + ";" + str(c) + ";" + str(k) + "\n")
    except:
        print("Fitted for " + str(value) + ": a=" + str(a) + ", b=" + str(b) + ", c=" + str(c) + ", k=" + str(k) + "\n")
    return (a, b, c, 0, k)


if __name__ == "__main__":
    PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-upravene/all/5kV_105_1_u.png"
    VALUE = 80
    VISUALIZE = True
    ANGLE = -11

    global img
    img = cv.imread(PATH, cv.IMREAD_GRAYSCALE)
    img = preprocessing(img, ANGLE)
    imgcopy = img.copy()
    img: np.ndarray = processing(img, VALUE)  # prepare visualizing image

    # shap = img.shape
    # img.resize((shap[0]+150, shap[1]), refcheck=False)
    res = fit_ellipse(imgcopy, VALUE, "ba.png")

    # prepare final image

    # draw final image
    draw_ellipse(img, *res)
    cv.imshow("fitted", img)
    cv.waitKey(0)
    cv.destroyAllWindows()
