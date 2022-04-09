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


@jit(nopython=True)
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


@jit(nopython=True)
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


@jit(nopython=True)
def rank_point(ellipsePoints: np.ndarray, x: int, y: int):
    lowestDist = 99999999
    for xp, yp in ellipsePoints:
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


@jit(nopython=True)
def rank_ellipse(a, b, c, k, maskPoints, height, width):
    rank = 0
    ellipsePoints = raster_ellipse(a, b, c, k, height, width)
    for y, x in maskPoints:
        rank += rank_point(ellipsePoints, x, y)
    return rank


@jit(nopython=True)
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


@jit(nopython=True)
def fit_a(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a - STEP_A, b, c, k, points, height, width)  # - to promote lower a
    if lastRank < currRank:
        DIR_A = UP
    else:
        DIR_A = DOWN
        a -= STEP_A

    while True:
        lastRank = currRank
        if DIR_A == UP:
            currRank = rank_ellipse(a + STEP_A, b, c, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                a += STEP_A
                if a > width:
                    break
        else:
            currRank = rank_ellipse(a - STEP_A, b, c, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                a -= STEP_A
                if a < STEP_A:
                    break

    # if VISUALIZE:
    #     show(a, b, c, k)
    # return a


@jit(nopython=True)
def fit_b(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a, b + STEP_B, c, k, points, height, width)
    if lastRank < currRank:
        DIR_B = DOWN
    else:
        DIR_B = UP
        b += STEP_B

    while True:
        lastRank = currRank
        if DIR_B == UP:
            currRank = rank_ellipse(a, b + STEP_B, c, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                b += STEP_B
                if b > height:
                    break
        else:
            currRank = rank_ellipse(a, b - STEP_B, c, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                b -= STEP_B
                if b < STEP_B:
                    break

        if VISUALIZE:
            show(a, b, c, k)
    return b


@jit(nopython=True)
def fit_c(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a, b, c + STEP_C, k, points, height, width)
    if lastRank < currRank:
        DIR_C = DOWN
    else:
        DIR_C = UP
        c += STEP_C

    while True:
        lastRank = currRank
        if DIR_C == UP:
            currRank = rank_ellipse(a, b, c + STEP_C, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                c += STEP_C
                if c > 1.5:
                    break
        else:
            currRank = rank_ellipse(a, b, c - STEP_C, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                if c - STEP_C <= -0.95:
                    break
                c -= STEP_C

        if VISUALIZE:
            show(a, b, c, k)

    return c


@jit(nopython=True)
def fit_k(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a, b, c, k + STEP_K, points, height, width)
    if lastRank < currRank:
        DIR_K = DOWN
    else:
        DIR_K = UP
        k += STEP_K

    while True:
        lastRank = currRank
        if DIR_K == UP:
            currRank = rank_ellipse(a, b, c, k + STEP_K, points, height, width)
            if lastRank < currRank:
                break
            else:
                k += STEP_K
                if k > height:
                    break
        else:
            currRank = rank_ellipse(a, b, c, k - STEP_K, points, height, width)
            if lastRank < currRank:
                break
            else:
                k -= STEP_K
                if k < -height:
                    break

        if VISUALIZE:
            show(a, b, c, k)

    return k


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


def fit_ellipse(img: np.ndarray, value: int = None, name: str = "") -> Tuple[float, float, float, float]:
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


def preprocessing(img, angle=-11, size=(200, 200)):
    img = ndimage.rotate(img, angle)
    img = cv.resize(img, size)
    return img


if __name__ == "__main__":
    PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-upravene/all/5kV_105_1_u.png"
    VALUE = 23
    VISUALIZE = True
    ANGLE = -11

    global img
    img = cv.imread(PATH, cv.IMREAD_GRAYSCALE)
    img = preprocessing(img, ANGLE)
    imgcopy = img.copy()
    img: np.ndarray = processing(img, VALUE)  # prepare visualizing image

    shap = img.shape
    img.resize((shap[0]+150, shap[1]), refcheck=False)
    res = fit_ellipse(imgcopy, VALUE, "ba.png")

    # prepare final image

    # draw final image
    draw_ellipse(img, *res)
    cv.imshow("fitted", img)
    cv.waitKey(0)
    cv.destroyAllWindows()
