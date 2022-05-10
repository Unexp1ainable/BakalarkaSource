"""
Author: Samuel Repka
Date: May 2022
Description: A library for the general ellipse operations.
"""
from typing import List, Tuple
from scipy import ndimage
from skimage.morphology import skeletonize
from numba import jit
import numpy as np
import cv2 as cv


def draw_ellipse(img: np.ndarray, a: float, b: float, c: float, h: float, k: float, color=255) -> None:
    """Draw an ellipse on an image

    Args:
        img (np.ndarray): Image to be drawn on
        a (float): a parameter
        b (float): b parameter
        c (float): c parameter
        h (float): h parameter
        k (float): k parameter
        color (int, optional): Line color. Defaults to 255.
    """
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
def rootsY(a: float, b: float, c: float, h: float, k: float, x: float) -> Tuple[int, float, float]:
    """Calculate roots of the superellipse defined by parameters in the x level

    Args:
        a (float): superellipse parameter
        b (float): superellipse parameter
        c (float): superellipse parameter
        h (float): superellipse parameter
        k (float): superellipse parameter
        x (float): superellipse parameter

    Returns:
        Tuple[int, float, float]: First number is the count of results, the rest are results. If the count is less than the number of results, the results are invalid and 0.
    """
    mid = 1-abs((x+h)/a)**(c)
    if mid < 0:
        return (0, 0., 0.)
    if mid == 0:
        return (1, -b-k, 0.)
    else:
        sD = b*(mid**(1/c))
        return (2, (sD-b-k), (-sD-b-k))


@jit(nopython=True)
def rootsX(a: float, b: float, c: float, h: float, k: float, y: float) -> Tuple[int, float, float]:
    """Calculate roots of the superellipse defined by parameters in the y level

    Args:
        a (float): superellipse parameter
        b (float): superellipse parameter
        c (float): superellipse parameter
        h (float): superellipse parameter
        k (float): superellipse parameter
        x (float): superellipse parameter

    Returns:
        Tuple[int, float, float]: First number is the count of results, the rest are results. If the count is less than the number of results, the results are invalid and 0.
    """
    mid = 1-abs((y+k+b)/b)**(c)
    if mid < 0:
        return (0, 0., 0.)
    if mid == 0:
        return (1, -h, 0.)
    else:
        sD = a*(mid**(1/c))
        return (2, (sD-h), (-sD-h))


@jit(nopython=True)
def raster_ellipse(a: float, b: float, c: float, h: float, k: float) -> np.ndarray:
    """Raster the superellipse

    Args:
        a (float): parameter of the superellipse
        b (float): parameter of the superellipse
        c (float): parameter of the superellipse
        h (float): parameter of the superellipse
        k (float): parameter of the superellipse

    Returns:
        np.ndarray: calculated points
    """
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
def rank_point(ellipsePoints: np.ndarray, x: int, y: int) -> float:
    """Return the squared distance to the closest point of the superellipse

    Args:
        ellipsePoints (np.ndarray): rasterized superellipse
        x (int): point coordinate
        y (int): point coordinate

    Returns:
        float: squared distance to the closest point
    """
    lowestDist = 99999999.
    for xp, yp in ellipsePoints:
        yp = -yp
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


@jit(nopython=True)
def rank_ellipse(
        a: float, b: float, c: float, h: float, k: float, maskPoints: List[Tuple[float, float]],
        height: float, width: float):
    """Return sum of least squares of the distances from each of the mask points to the superellipse

    Args:
        a (float): superellipse parameter
        b (float): superellipse parameter
        c (float): superellipse parameter
        h (float): superellipse parameter
        k (float): superellipse parameter
        maskPoints (List[Tuple[float, float]]): list of points for which the rank is calculated
        height (float): size of the image
        width (float): size of the image

    Returns:
        _type_: _description_
    """
    rank = 0
    hx = width//2
    ellipsePoints = raster_ellipse(a, b, c, h, k)
    for y, x in maskPoints:
        x -= hx
        rank += rank_point(ellipsePoints, x, y)
    return rank


@jit(nopython=True)
def get_bounds(img: np.ndarray) -> Tuple[int, int, int, int]:
    """Strip borders of the image containing just 0 pixels

    Args:
        img (np.ndarray): image to be filtered

    Returns:
        Tuple[int,int,int,int]: Minimal rectangular area of the image where all of the non-zero pixels are.
    """
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


def preprocessing(img: np.ndarray, angle=-11, size=(1024, 1024)) -> np.ndarray:
    """Rotate and scale image

    Args:
        img (np.ndarray): Image to be processed
        angle (int, optional): Angle of rotation in degrees. Defaults to -11.
        size (tuple, optional): Final size after scaling. Defaults to (1024, 1024).

    Returns:
        np.ndarray: Processed image
    """
    img = ndimage.rotate(img, angle, reshape=False)
    img = cv.resize(img, size)
    return img


def processing(img: np.ndarray, value: int) -> np.ndarray:
    """Filter pixels with given value from the image, skeletonize result and remove lone pixels.

    Args:
        img (np.ndarray): Image to be processed
        value (int): Pixel value to be filtered

    Returns:
        np.ndarray: Processed image
    """

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
