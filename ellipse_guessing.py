from math import isclose, sqrt
from multiprocessing.synchronize import Lock
from typing import List, Tuple
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from scipy import ndimage
from skimage.morphology import skeletonize

UP = True
DOWN = False

STEP_A = 1
STEP_B = 1
STEP_C = 0.05
STEP_K = 0.5

DIR_A = None
DIR_B = None
DIR_C = None
DIR_K = None

MAX_ITERATIONS = 25


def draw_ellipse(img: np.ndarray, a: float, b: float, c: float, k: float) -> None:
    A = c
    B = a**2
    C = -2*k*b*c+k**2*c+b**2*c+b**2
    D = -2*c*k+2*b*c
    E = -2*k*a**2+2*b*a**2
    F = -2*k*b*a**2+k**2*a**2

    hx = img.shape[1]//2

    ystart = 0
    yend = img.shape[0]-1
    xstart = int((img.shape[1]-a*2)//2)
    xend = round(xstart+a*2)+1
    if xstart < 0:
        xstart = 0
    if xend >= img.shape[1]:
        xend = img.shape[1]

    for y in range(ystart, -yend, -1):
        try:
            xs = np.roots((A*y**2+C+D*y, 0, E*y+B*y**2+F))
        except:
            continue

        if len(xs) < 1 or np.iscomplex(xs)[0]:
            continue
        x = round(xs[0])+hx
        if xstart <= x < xend:
            img[-y][x] = 255

        if len(xs) == 1:
            continue
        x = round(xs[1])+hx
        if xstart <= x < xend:
            img[-y][x] = 255

    for x in range(xstart, xend):
        x -= hx
        try:
            ys = np.roots((B+A*x**2, D*x**2+E, C*x**2+F))
        except:
            continue

        if len(ys) < 1 or np.iscomplex(ys)[0]:
            continue
        y = -round(ys[0])
        if ystart <= y < yend:
            img[y][x+hx] = 255

        if len(ys) == 1:
            continue
        y = -round(ys[1])
        if ystart <= y < yend:
            img[y][x+hx] = 255


def raster_ellipse(a: float, b: float, c: float, k: float, height: int, width: int):
    result = []
    A = c
    B = a**2
    C = -2*k*b*c+k**2*c+b**2*c+b**2
    D = -2*c*k+2*b*c
    E = -2*k*a**2+2*b*a**2
    F = -2*k*b*a**2+k**2*a**2

    hx = width//2

    ystart = 0
    yend = height
    xstart = int((width-a*2)//2)
    xend = round(xstart+a*2)

    for y in range(ystart, -yend, -1):
        try:
            xs = np.roots((A*y**2+C+D*y, 0, E*y+B*y**2+F))
        except:
            continue

        if len(xs) < 1 or np.iscomplex(xs)[0]:
            continue
        x = round(xs[0])+hx
        if xstart <= x < xend:
            result.append((x, -y))

        if len(xs) == 1:
            continue
        x = round(xs[1])+hx
        if xstart <= x < xend:
            result.append((x, -y))

    for x in range(xstart, xend):
        x -= hx
        try:
            ys = np.roots((B+A*x**2, D*x**2+E, C*x**2+F))
        except:
            continue

        if len(ys) < 1 or np.iscomplex(ys)[0]:
            continue
        y = -round(ys[0])
        if ystart <= y < yend and (x+hx, y) not in result:
            result.append((x+hx, y))

        if len(ys) == 1:
            continue
        y = -round(ys[1])
        if ystart <= y < yend and (x+hx, y) not in result:
            result.append((x+hx, y))
    return result


def rank_point(ellipsePoints: List[Tuple[int, int]], x: int, y: int):
    lowestDist = 99999999
    for xp, yp in ellipsePoints:
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


def rank_ellipse(a, b, c, k, maskPoints, height, width):
    rank = 0
    ellipsePoints = raster_ellipse(a, b, c, k, height, width)
    for y, x in maskPoints:
        rank += rank_point(ellipsePoints, x, y)
    return rank


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


def fit_a(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width) -> Tuple[float, float, float, float]:
    lastRank = rank_ellipse(a, b, c, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a + STEP_A, b, c, k, points, height, width)
    if lastRank < currRank:
        DIR_A = DOWN
    else:
        DIR_A = UP
        a += STEP_A

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

        # dimg = img.copy()
        # draw_ellipse(dimg, a, b, c, k)
        # cv.imshow("tmp", dimg)
        # cv.waitKey(1)

    return a


def fit_b(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width) -> Tuple[float, float, float, float]:
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
        else:
            currRank = rank_ellipse(a, b - STEP_B, c, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                b -= STEP_B

        # dimg = img.copy()
        # draw_ellipse(dimg, a, b, c, k)
        # cv.imshow("tmp", dimg)
        # cv.waitKey(1)

    return b


def fit_c(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width) -> Tuple[float, float, float, float]:
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
        else:
            currRank = rank_ellipse(a, b, c - STEP_C, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                c -= STEP_C

        # dimg = img.copy()
        # draw_ellipse(dimg, a, b, c, k)
        # cv.imshow("tmp", dimg)
        # cv.waitKey(1)

    return c


def fit_k(a: float, b: float, c: float, k: float, points: List[Tuple[int, int]],
          height, width) -> Tuple[float, float, float, float]:
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
        else:
            currRank = rank_ellipse(a, b, c, k - STEP_K, points, height, width)
            if lastRank < currRank:
                break
            else:
                k -= STEP_K

        # dimg = img.copy()
        # draw_ellipse(dimg, a, b, c, k)
        # cv.imshow("tmp", dimg)
        # cv.waitKey(1)

    return k


def fit_ellipse(img: np.ndarray, value: int = None) -> Tuple[float, float, float, float]:
    global lock
    if type(img) == tuple:  # multiprocessing arguments
        value = img[1]
        img = img[0]

    print("Started with value ", value)
    if value is not None:
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

    bounds = get_bounds(img)
    if bounds[0] >= bounds[2] or bounds[1] >= bounds[3]:
        print("Error, no pixels of interest. " + str(value))
        exit(1)

    points = np.argwhere(img)

    a = round((bounds[2]-bounds[0])*0.5)
    b = round((bounds[3]-bounds[1])*0.5 + 10)  # +10 to prefer bigger ellipses
    c = 0
    k = -bounds[1]

    convergence = False
    i = 0

    height = img.shape[0]
    width = img.shape[1]

    while not convergence:
        oldb = b
        olda = a
        oldc = c
        oldk = k
        a = fit_a(a, b, c, k, points, height, width)
        b = fit_b(a, b, c, k, points, height, width)
        c = fit_c(a, b, c, k, points, height, width)
        k = fit_k(a, b, c, k, points, height, width)
        # print(a, b, c, k)

        convergence = isclose(olda, a) and isclose(oldb, b, abs_tol=0.001) and isclose(oldc, c) and isclose(oldk, k)
        if i >= MAX_ITERATIONS:
            break
        i += 1

    print("convergence reached " + str(value))
    draw_ellipse(img, a, b, c, k)
    cv.imwrite("output/img" + str(value) + ".png", img)

    try:
        with open("output.csv", "a") as file:
            file.write(str(value) + ";" + str(a) + ";" + str(b) + ";" + str(c) + ";" + str(k) + "\n")
    except:
        print("Fitted for " + str(value) + ": a=" + str(a) + ", b=" + str(b) + ", c=" + str(c) + ", k=" + str(k) + "\n")
    return (a, b, c, k)


if __name__ == "__main__":
    PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-5k-upravene/5kV_105_1_u.png"
    VALUE = 196

    global img
    img = cv.imread("asc.png", cv.IMREAD_GRAYSCALE)
    img = ndimage.rotate(img, -8)
    img = cv.resize(img, (200, 200))

    pimg = img.copy()
    img = img == VALUE
    img = skeletonize(img)
    img = img.astype(np.uint8)

    # filter lone pixels
    kernelsize = 11
    neighbourPixelCount = 2
    kernel = np.ones((kernelsize, kernelsize), np.uint8)
    kernel[kernelsize//2][kernelsize//2] = kernelsize**2
    img = cv.filter2D(img, -1, kernel)
    _, img = cv.threshold(img, kernelsize**2+neighbourPixelCount-1, 255, cv.THRESH_BINARY)
    res = fit_ellipse(pimg, VALUE)

    img = img == VALUE
    img = img.astype(np.uint8) * 255
    draw_ellipse(img, *res)
    cv.imshow("fitted", img)
    cv.waitKey(0)
    cv.destroyAllWindows()
