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


# @jit(nopython=True)
def fit_a(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:

    pts = [
        (0.0, 482.0),
        (15.0, 484.0),
        (30.0, 484.0),
        (45.0, 490.0),
        (60.0, 494.0),
        (75.0, 500.0),
        (90.0, 512.0),
        (105.0, 514.0),
        (120.0, 500.0),
        (135.0, 486.0),
        (150.0, 456.0),
        (165.0, 424.0),
        (180.0, 386.0),
        (195.0, 324.0),
        (210.0, 208.0),
        (225.0, 0.0),
        (240.0, 0.0),
        (255.0, 0.0)]
    cs = PchipInterpolator(list(range(0, 256, 15)), pts)
    return cs(val)[1]


# @jit(nopython=True)
def fit_b(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> float:

    pts = [
        (0.0, 667.1042864583333),
        (7.5, 667.1042864583333),
        (15.0, 667.1042864583333),
        (22.5, 667.1042864583333),
        (30.0, 664.4984103393554),
        (37.75, 667.1042864583333),
        (45.25, 667.1042864583333),
        (52.75, 626.7132066141763),
        (60.25, 525.0840379740397),
        (67.75, 419.54605515543614),
        (75.25, 347.8844618835449),
        (82.75, 328.34039099121094),
        (90.25, 316.61394845581054),
        (98.0, 307.493382039388),
        (105.5, 299.6757536824544),
        (113.0, 290.55518726603185),
        (120.5, 285.34343502807616),
        (128.0, 284.0404969685872),
        (135.5, 284.0404969685872),
        (143.0, 280.1316827901204),
        (150.5, 272.3140544331868),
        (158.0, 261.89054995727537),
        (165.75, 244.95235518391925),
        (173.25, 225.40828429158526),
        (180.75, 201.9553992207845),
        (188.25, 182.4113283284505),
        (195.75, 155.04962907918292),
        (203.25, 123.77911565144856),
        (210.75, 80.7821596883138),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/34)), pts)
    return cs(val)[1]


# @jit(nopython=True)
def fit_c(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    return -0.3


# @jit(nopython=True)
def fit_k(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    pts = [
        (0.0, -0.0),
        (7.5, -0.0),
        (15.0, -0.0),
        (22.5, -0.0),
        (30.0, -0.0),
        (37.75, -0.0),
        (45.25, -0.0),
        (52.75, -1.7382812499999998),
        (60.25, -10.429687499999998),
        (67.75, -43.45703124999999),
        (75.25, -81.69921874999999),
        (82.75, -118.20312499999999),
        (90.25, -147.75390625),
        (98.0, -187.73437499999997),
        (105.5, -220.76171874999997),
        (113.0, -265.95703125),
        (120.5, -297.24609375),
        (128.0, -325.05859374999994),
        (135.5, -358.08593749999994),
        (143.0, -389.37499999999994),
        (150.5, -429.35546874999994),
        (158.0, -460.64453124999994),
        (165.75, -497.14843749999994),
        (173.25, -533.65234375),
        (180.75, -584.0625),
        (188.25, -629.2578125),
        (195.75, -683.1445312499999),
        (203.25, -735.2929687499999),
        (210.75, -804.8242187499999),
        (218.25, -889.9999999999999),
        (226.0, -0.0),
        (233.5, -0.0),
        (241.0, -0.0),
        (248.5, -0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/34)), pts)
    return cs(val)[1]


def fit_ellipse(img: np.ndarray, value: int = None, name: str = "") -> Tuple[float, float, float, float]:
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
        return (0., 0., 0., 0.)

    points = np.argwhere(img)
    height = img.shape[0]
    width = img.shape[1]

    a = round((bounds[2]-bounds[0])*0.5)
    b = a/2
    c = 0
    k = -bounds[1]

    a = fit_a(a, b, c, k, points, height, width, value)
    b = fit_b(a, b, c, k, points, height, width, value)
    c = fit_c(a, b, c, k, points, height, width, value)
    k = fit_k(a, b, c, k, points, height, width, value)

    print("convergence reached " + str(value))
    draw_ellipse(img, a, b, c, k)

    outdir = "output/"
    if name:
        outdir = "output/" + name[:-4] + "/"

    cv.imwrite(outdir + "img" + str(value) + ".png", img)

    try:
        with open(outdir + "output.csv", "a") as file:
            file.write(str(value) + ";" + str(a) + ";" + str(b) + ";" + str(c) + ";" + str(k) + "\n")
    except:
        print("Fitted for " + str(value) + ": a=" + str(a) + ", b=" + str(b) + ", c=" + str(c) + ", k=" + str(k) + "\n")
    return (a, b, c, k)


if __name__ == "__main__":
    PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-upravene/all/5kV_105_1_u.png"
    VALUE = 120
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
