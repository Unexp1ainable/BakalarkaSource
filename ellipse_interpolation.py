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


def draw_ellipse(img: np.ndarray, a: float, b: float, c: float, k: float) -> None:
    w = img.shape[1]
    h = img.shape[0]
    points = raster_ellipse(a, b, c, k, h, w)
    if len(points) == 0:
        return
    for x, y in points:
        if 0 <= x < w and 0 <= y < h:
            img[y][x] = 255


# @jit(nopython=True)
def roots(a, b, c):
    if a == 0:
        return (0, 0., 0.)
    D = b**2-4*a*c
    if D < 0:
        return (0, 0., 0.)
    if D == 0:
        return (1, -b/(2*a), 0.)
    else:
        sD = sqrt(D)
        return (2, (-b+sD)/(2*a), (-b-sD)/(2*a))


# @jit(nopython=True)
def raster_ellipse(a: float, b: float, c: float, k: float, height: int, width: int):
    if a == 0 or b == 0:
        return np.empty((0, 0), np.int64)
    result = []
    A = c
    B = a**2
    C = -2*k*b*c+k**2*c+b**2*c+b**2
    D = -2*c*k+2*b*c
    E = -2*k*a**2+2*b*a**2
    F = -2*k*b*a**2+k**2*a**2

    hx = width//2

    ystart = round(k)
    yend = round(b*2) - ystart
    xstart = round(-a)
    xend = round(a)

    for y in range(ystart, -yend, -1):
        n, x1, x2 = roots(A*y**2+C+D*y, 0, E*y+B*y**2+F)

        if n < 1:
            continue
        x = round(x1)+hx
        result.append((x, -y))

        if n == 1:
            continue
        x = round(x2)+hx
        result.append((x, -y))

    for x in range(xstart, xend):
        n, y1, y2 = roots(B+A*x**2, D*x**2+E, C*x**2+F)

        if n < 1:
            continue
        y = -round(y1)
        if (x+hx, y) not in result:
            result.append((x+hx, y))

        if n == 1:
            continue
        y = -round(y2)
        if (x+hx, y) not in result:
            result.append((x+hx, y))
    return np.array(result, np.int64)


# @jit(nopython=True)
def rank_point(ellipsePoints: np.ndarray, x: int, y: int):
    lowestDist = 99999999
    for xp, yp in ellipsePoints:
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


# @jit(nopython=True)
def rank_ellipse(a, b, c, k, maskPoints, height, width):
    rank = 0
    ellipsePoints = raster_ellipse(a, b, c, k, height, width)
    for y, x in maskPoints:
        rank += rank_point(ellipsePoints, x, y)
    return rank


# @jit(nopython=True)
def get_bounds(img):
    height = img.shape[0]
    width = img.shape[1]
    up = 0
    down = height
    left = 0
    right = width
    for y in range(height):
        up = y
        if (img[y].any()):
            break

    for y in range(1, height+1):
        down = height - y - 1
        if (img[-y].any()):
            break

    timg = img.T
    for x in range(width):
        left = x
        if (timg[x].any()):
            break

    for x in range(1, width+1):
        right = width - x - 1
        if (timg[-x].any()):
            break

    return (left, up, right, down)


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


def processing(img, value):
    img = img == value
    img = skeletonize(img)

    img = img.astype(np.uint8)
    # filter lone pixels
    kernelsize = 11
    neighbourPixelCount = 2
    kernel = np.ones((kernelsize, kernelsize), np.uint8)
    kernel[kernelsize//2][kernelsize//2] = kernelsize**2
    img = cv.filter2D(img, -1, kernel)
    _, img = cv.threshold(img, kernelsize**2+neighbourPixelCount-1, 255, cv.THRESH_BINARY)
    return img


def fit_ellipse_old(img: np.ndarray, value: int = None, name: str = "") -> Tuple[float, float, float, float]:
    global STEP_A, STEP_B, STEP_C
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

    convergence = False
    i = 0

    while not convergence:
        oldb = b
        olda = a
        oldc = c
        oldk = k
        a = fit_a(a, b, c, k, points, height, width, value)
        b = fit_b(a, b, c, k, points, height, width, value)
        c = fit_c(a, b, c, k, points, height, width, value)
        k = fit_k(a, b, c, k, points, height, width, value)

        if a == 0:
            return
        # print(a, b, c, k)

        convergence = isclose(olda, a) and isclose(oldb, b, abs_tol=0.001) and isclose(oldc, c) and isclose(oldk, k)
        if i >= ROUGH_ITERATIONS:
            break
        i += 1

    convergence = False
    i = 0
    STEP_A /= 2
    STEP_B /= 2
    STEP_C /= 2
    while not convergence:
        oldb = b
        olda = a
        oldc = c
        oldk = k
        a = fit_a(a, b, c, k, points, height, width, value)
        b = fit_b(a, b, c, k, points, height, width, value)
        c = fit_c(a, b, c, k, points, height, width, value)
        k = fit_k(a, b, c, k, points, height, width, value)

        if a == 0:
            return
        # print(a, b, c, k)

        convergence = isclose(olda, a) and isclose(oldb, b, abs_tol=0.001) and isclose(oldc, c) and isclose(oldk, k)
        if i >= SMOOTH_ITERATIONS:
            break
        i += 1

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


def preprocessing(img, angle=-11, size=(1024, 1024)):
    img = ndimage.rotate(img, angle, reshape=False)
    img = cv.resize(img, size)
    return img


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
