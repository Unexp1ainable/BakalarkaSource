
from scipy import ndimage
from skimage.morphology import skeletonize
from numba import jit
from math import isclose, log, sqrt
import numpy as np
import cv2 as cv


def draw_ellipse(img: np.ndarray, a: float, b: float, c: float, h: float, k: float, color=255) -> None:
    wi = img.shape[1]
    hi = img.shape[0]
    hx = hi//2

    points = raster_ellipse(a, b, c, h, k)
    for x, y in points:
        x += hx
        y = -y
        if 0 <= x < wi and 0 <= y < hi:
            img[y][x] = color


@jit(nopython=True)
def rootsY(a, b, c, h, k, x):
    mid = 1-abs((x+h)/a)**(c)
    if mid < 0:
        return (0, 0., 0.)
    if mid == 0:
        return (1, -b-k, 0.)
    else:
        sD = b*(mid**(1/c))
        return (2, (sD-b-k), (-sD-b-k))


@jit(nopython=True)
def rootsX(a, b, c, h, k, y):
    mid = 1-abs((y+k+b)/b)**(c)
    if mid < 0:
        return (0, 0., 0.)
    if mid == 0:
        return (1, -h, 0.)
    else:
        sD = a*(mid**(1/c))
        return (2, (sD-h), (-sD-h))


@jit(nopython=True)
def raster_ellipse(a: float, b: float, c: float, h: float, k: float):
    if a == 0 or b == 0:
        return np.empty((0, 0), np.int64)
    result = []

    ystart = -round(k)
    yend = round(b*2) - ystart
    xstart = round(-a-h)
    xend = round(a-h)

    for y in range(ystart, -yend, -1):
        n, x1, x2 = rootsX(a, b, c, h, k, y)

        if n < 1:
            continue
        x = round(x1)
        result.append((x, y))

        if n == 1:
            continue
        x = round(x2)
        result.append((x, y))

    for x in range(xstart, xend):
        n, y1, y2 = rootsY(a, b, c, h, k, x)

        if n < 1:
            continue
        y = round(y1)
        if (x, y) not in result:
            result.append((x, y))

        if n == 1:
            continue
        y = round(y2)
        if (x, y) not in result:
            result.append((x, y))

    return np.array(result, np.int64)


@jit(nopython=True)
def rank_point(ellipsePoints: np.ndarray, x: int, y: int):
    lowestDist = 99999999
    for xp, yp in ellipsePoints:
        yp = -yp
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


@jit(nopython=True)
def rank_ellipse(a, b, c, h, k, maskPoints, height, width):
    rank = 0
    hx = width//2
    ellipsePoints = raster_ellipse(a, b, c, h, k)
    for y, x in maskPoints:
        x -= hx
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


if __name__ == "__main__":
    imze = np.zeros((300, 300), np.uint8)
    draw_ellipse(imze, 80, 100, 2, -50, 20)
    cv.imshow("a", imze)
    cv.waitKey(0)
    cv.destroyAllWindows()
