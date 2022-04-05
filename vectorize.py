from ellipse_guessing import fit_ellipse
import multiprocessing as mp
import cv2 as cv
from scipy import ndimage
import numpy as np


def args(img):
    for i in range(1, 256):
        yield(img, i)


if __name__ == "__main__":
    img = cv.imread(
        "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-5k-upravene/5kV_105_1_u.png", cv.IMREAD_GRAYSCALE)
    img = ndimage.rotate(img, -8)
    img = cv.resize(img, (200, 200))

    lock = mp.Lock()
    with open("output.csv", "w") as f:
        f.write("--- algorithm has started ---\n")
        f.write("value;a;b;c;k\n")
    pool = mp.Pool()
    pool.map(fit_ellipse, args(img))
    pool.close()
    pool.join()
    print("All processes have finished.")
    with open("output.csv", "a") as f:
        f.write("--- finished ---\n")
