import cv2 as cv
import numpy as np
import argparse


# Q4 is baseline
PATH_BASELINE = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/Q4.png"

PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/Q1.png"
# PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/Q2.png"
# PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/Q3.png"

WNAME = "Image"

global alpha, beta, im_base, im_adjust
alpha = 1.0
beta = 0
im_base = None
im_adjust = None


def mouseCallback(event, x, y, flags, userdata):
    global alpha, beta
    if event == cv.EVENT_MOUSEMOVE and flags & cv.EVENT_FLAG_LBUTTON:
        w = im_base.shape[1]
        h = im_base.shape[1]
        alpha = x/w*2
        beta = y/h*254-127

        dst = cv.convertScaleAbs(im_adjust, None, alpha, beta)
        img = np.concatenate((im_base, dst))
        cv.imshow(WNAME, img)


if __name__ == "__main__":
    im_base = cv.imread(PATH_BASELINE, cv.IMREAD_GRAYSCALE)
    if im_base is None:
        print('Could not open or find the image: ', PATH_BASELINE)
        exit(0)
    im_adjust = cv.imread(PATH, cv.IMREAD_GRAYSCALE)
    if im_adjust is None:
        print('Could not open or find the image: ', PATH)
        exit(0)
    if im_base.shape != im_adjust.shape:
        print("Image sizes are different.")
        exit(0)

    im_base = im_base[:im_base.shape[1]//2-1]
    im_adjust = im_adjust[im_adjust.shape[1]//2-1:im_base.shape[1]]
    img = np.concatenate((im_base, im_adjust))

    cv.namedWindow(WNAME, cv.WINDOW_KEEPRATIO)
    cv.imshow(WNAME, img)
    cv.setMouseCallback(WNAME, mouseCallback)

    while True:
        key = cv.waitKey(10)
        if key == 27:  # esc
            break
        if key == 13:  # enter
            print("Alpha: ", alpha, ", Beta: ", beta)
        # close on manual window close
        if cv.getWindowProperty(WNAME, cv.WND_PROP_VISIBLE) < 1:
            break

    cv.destroyAllWindows()
