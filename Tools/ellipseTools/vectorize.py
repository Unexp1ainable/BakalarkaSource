from os import mkdir
import multiprocessing as mp
import cv2 as cv
from scipy import ndimage
import numpy as np
from ellipse_fitting import fit_ellipse, preprocessing

FILE = "list.txt"


def args(img, name):
    """Generator of arguments when dispatching jobs

    Args:
        img (np.ndarray): Image to be processed
        name (str): name of the image
    """
    for i in range(1, 256):
        yield(img, i, name)


if __name__ == "__main__":
    with open(FILE) as f:
        for line in f:
            name = line.strip()
            outdir = "output/" + name[:-4] + "/"
            try:
                mkdir(outdir)
            except:
                pass

            img: np.ndarray = cv.imread(dir+name, cv.IMREAD_GRAYSCALE)
            if img is None:
                print("Image does not exist.")
                exit(1)

            # prepare image
            img = preprocessing(img, size=(1024, 1024), angle=151)
            # prepare output file (no kind of synchronisation is made when processes write into it, sometimes it breaks)
            with open(outdir + "output.csv", "w") as f:
                f.write("--- algorithm has started ---\n")
                f.write("value;a;b;c;k\n")

            # dispatch jobs
            pool = mp.Pool()
            pool.map(fit_ellipse, args(img, name))
            pool.close()
            pool.join()
            print("All processes have finished.")
            with open(outdir + "output.csv", "a") as f:
                f.write("--- finished ---\n")
