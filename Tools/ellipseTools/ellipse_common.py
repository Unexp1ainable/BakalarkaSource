
from scipy import ndimage
from skimage.morphology import skeletonize
from numba import jit
from math import isclose, log, sqrt
import numpy as np
import cv2 as cv


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


def preprocessing(img, angle=-11, size=(1024, 1024)):
    img = ndimage.rotate(img, angle, reshape=False)
    img = cv.resize(img, size)
    return img


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
