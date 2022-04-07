from os import mkdir
from ellipse_guessing import fit_ellipse
import multiprocessing as mp
import cv2 as cv
from scipy import ndimage
import numpy as np


def args(img, name):
    for i in range(1, 256):
        yield(img, i, name)


if __name__ == "__main__":
    dir = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-upravene/all/"
    name = "5kV_105_1_u.png"
    outdir = "output/" + name[:-4] + "/"
    try:
        mkdir(outdir)
    except:
        pass

    img: np.ndarray = cv.imread(dir+name, cv.IMREAD_GRAYSCALE)
    if img is None:
        print("Image does not exist.")
    else:
        img = ndimage.rotate(img, -8)
        img = cv.resize(img, (200, 200))

        lock = mp.Lock()
        with open(outdir + "output.csv", "w") as f:
            f.write("--- algorithm has started ---\n")
            f.write("value;a;b;c;k\n")
        pool = mp.Pool()
        pool.map(fit_ellipse, args(img, name))
        pool.close()
        pool.join()
        print("All processes have finished.")
        with open(outdir + "output.csv", "a") as f:
            f.write("--- finished ---\n")
