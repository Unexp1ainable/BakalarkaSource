from copy import deepcopy
import cv2 as cv
import numpy as np

KEY_ESC = 27


def mouseCallback(event, x, y, flags, params):
    if event == cv.EVENT_LBUTTONDOWN:
        im = params
        colorPixels(im[y][x])


def colorPixels(value: int) -> None:
    for i in range(4):
        showIms[i] = deepcopy(templates[i])
        showIms[i][ims[i] == value] = (255, 0, 0)


if __name__ == "__main__":
    global showIms, ims, templates
    img = cv.imread("sim.png", cv.IMREAD_GRAYSCALE)
    ims = [img]

    for i in range(3):
        im = ims[i].copy()
        cv.rotate(im, cv.ROTATE_90_CLOCKWISE, im)
        ims.append(im)

    showIms = deepcopy(ims)
    for i in range(4):
        showIms[i] = cv.cvtColor(showIms[i], cv.COLOR_GRAY2BGR)
        wname = "img" + str(i)
        cv.namedWindow(wname)
        cv.setMouseCallback(wname, mouseCallback, ims[i])

    templates = deepcopy(showIms)

    while True:
        for i in range(4):
            cv.imshow("img" + str(i), showIms[i])
        key = cv.waitKey(20)

        if key == KEY_ESC:
            break
    cv.destroyAllWindows()
