from math import sqrt
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt


def draw_ellipse(img: np.ndarray, a: float, b: float, c: float, k: float) -> None:
    A = b**2+c*k**2
    B = a**2
    C = c
    D = -2*c*k
    E = -2*k*a**2
    F = k**2*a**2-a**2*b**2

    hx = img.shape[0]//2
    hy = img.shape[1]//2

    ystart = int((img.shape[1]-b*2)//2)
    yend = round(ystart+b*2)
    xstart = int((img.shape[0]-a*2)//2)
    xend = round(xstart+a*2)

    for y in range(ystart, yend):
        y -= hy
        try:
            xs = np.roots((A+C*y**2+D*y, 0, E*y+B*y**2+F))
        except:
            continue

        if len(xs) < 1 or np.iscomplex(xs)[0]:
            continue
        x = round(xs[0])+hx
        if xstart <= x <= xend:
            img[y+hy][x] = 255

        if len(xs) == 1:
            continue
        x = round(xs[1])+hx
        if xstart <= x <= xend:
            img[y+hy][x] = 255

    for x in range(xstart, xend):
        x -= hx
        try:
            ys = np.roots((B+C*x**2, D*x**2+E, A*x**2+F))
        except:
            continue

        if len(ys) < 1 or np.iscomplex(ys)[0]:
            continue
        y = round(ys[0])+hy
        if ystart <= y <= yend:
            img[y][x+hx] = 255

        if len(ys) == 1:
            continue
        y = round(ys[1])+hy
        if ystart <= y <= yend:
            img[y][x+hx] = 255


def raster_ellipse(img: np.ndarray, a: float, b: float, c: float, k: float):
    result = []
    A = b**2+c*k**2
    B = a**2
    C = c
    D = -2*c*k
    E = -2*k*a**2
    F = k**2*a**2-a**2*b**2

    hx = img.shape[0]//2
    hy = img.shape[1]//2

    ystart = int((img.shape[1]-b*2)//2)
    yend = round(ystart+b*2)
    xstart = int((img.shape[0]-a*2)//2)
    xend = round(xstart+a*2)

    for y in range(ystart, yend):
        y -= hy
        try:
            xs = np.roots((A+C*y**2+D*y, 0, E*y+B*y**2+F))
        except:
            continue

        if len(xs) < 1 or np.iscomplex(xs)[0]:
            continue
        x = round(xs[0])+hx
        if xstart <= x <= xend:
            result.append((x, y+hy))

        if len(xs) == 1:
            continue
        x = round(xs[1])+hx
        if xstart <= x <= xend:
            result.append((x, y+hy))

    for x in range(xstart, xend):
        x -= hx
        try:
            ys = np.roots((B+C*x**2, D*x**2+E, A*x**2+F))
        except:
            continue

        if len(ys) < 1 or np.iscomplex(ys)[0]:
            continue
        y = round(ys[0])+hy
        if ystart <= y <= yend and (x+hx, y) not in result:
            result.append((x+hx, y))

        if len(ys) == 1:
            continue
        y = round(ys[1])+hy
        if ystart <= y <= yend and (x+hx, y) not in result:
            result.append((x+hx, y))
    return result


def rank_point(ellipsePoints, x, y):
    lowestDist = 99999999
    for xp, yp in points:
        d = (x-xp)**2+(y-yp)**2
        if d < lowestDist:
            lowestDist = d
    return lowestDist


def rank_ellipse(ellipsePoints, maskPoints):
    rank = 0
    for x, y in maskPoints:
        rank += rank_point(ellipsePoints, x, y)
    return rank


def mouse_callback(event, x, y, flags, userdata):
    if event != cv.EVENT_LBUTTONDOWN:
        return
    global points
    global point, cpoint
    lowest = (9999999, 9999999)
    lowestDist = 99999999
    for xp, yp in points:
        d = sqrt((x-xp)**2+(y-yp)**2)
        if d < lowestDist:
            lowest = (xp, yp)
            lowestDist = d

    point = lowest
    cpoint = (x, y)
    print(x, y)


if __name__ == "__main__":
    img = np.zeros((1000, 1000), np.uint8)
    draw_ellipse(img, 300, 100, -0.5, 0)
    global points, point, cpoint
    point = (0, 0)
    cpoint = (0, 0)
    points = raster_ellipse(img, 300, 100, -0.5, 0)
    # plt.scatter(*zip(*points))
    # plt.show()
    showImg = img.copy()
    cv.imshow("ellipse", showImg)
    cv.setMouseCallback("ellipse", mouse_callback)
    while (True):
        showImg = img.copy()
        cv.ellipse(showImg, point, (20, 20), 0, 0, 0, 122, 10)
        cv.ellipse(showImg, cpoint, (20, 20), 0, 0, 0, 122, 10)
        cv.imshow("ellipse", showImg)

        key = cv.waitKey(100)
        if key == 27:  # esc
            break

    cv.destroyAllWindows()
