"""
Author: Samuel Repka
Date: May 2022
Description: A tool for automatic detection of calibration parameters for the offset and gain operation relative to the base image.
"""

import cv2 as cv
import numpy as np
from skimage.exposure import match_histograms
import matplotlib.pyplot as plt


# Q4 is the baseline
PATH_BASELINE = "dataset/calibration/5kV_10mm/Q4.png"
PATHS_TO_CALIBRATE = [("Q1", "dataset/calibration/5kV_10mm/Q1.png"),
                      ("Q2", "dataset/calibration/5kV_10mm/Q2.png"),
                      ("Q3", "dataset/calibration/5kV_10mm/Q3.png"), ]

if __name__ == "__main__":
    im_base = cv.imread(PATH_BASELINE, cv.IMREAD_GRAYSCALE)
    h1 = cv.calcHist([im_base], [0], None, [256], [0, 256])
    # remove black background
    h1[0] = 0
    # noramlise
    h1 = h1/np.linalg.norm(h1)

    if im_base is None:
        print('Could not open or find the image: ', PATH_BASELINE)
        exit(0)
    for name, path in PATHS_TO_CALIBRATE:
        im_adjust = cv.imread(path, cv.IMREAD_GRAYSCALE)
        if im_adjust is None:
            print('Could not open or find the image: ', path)
            exit(0)
        if im_base.shape != im_adjust.shape:
            print("Image sizes are different.")
            exit(0)

        plt.plot(h1, label="Segment 1")
        h2 = cv.calcHist([im_adjust], [0], None, [256], [0, 256])
        # remove black background
        h2[0] = 0
        # calculate offset with the global maximas
        offset = np.argmax(h1) - np.argmax(h2)
        gain = 1

        # calculate gain with the highest non-zero bucket of the histogram
        for i in range(10):
            gain = np.max(np.nonzero(h1)) / (np.max(np.nonzero(h2)+offset))
            offset = np.argmax(h1) - np.argmax(h2)*gain

        im_res = im_adjust.astype(np.float64)
        im_res *= gain
        im_res += offset
        # clip values
        im_res[im_res < 0] = 0
        im_res[im_res > 255] = 255
        im_res: np.ndarray

        # without this noise, a periodic peaks could be created due to rounding
        im_res += np.random.random(im_res.shape)-0.5
        im_adjust = im_res.round().astype(np.uint8)

        print(f"Segment {name}- Contrast: {gain}, Brightness: {offset}")

        h = cv.calcHist([im_adjust], [0], None, [256], [0, 256])
        if offset < 0:
            h[0] = 0
        else:
            h[int(offset)] = 0
            h[round(offset)] = 0
        h = h/np.linalg.norm(h)

        plt.plot(h, label="Segment 2")
        plt.legend()
        plt.show()
