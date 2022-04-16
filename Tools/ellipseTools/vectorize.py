from os import mkdir
import multiprocessing as mp
import cv2 as cv
from scipy import ndimage
import numpy as np

INTERPOLATE = True
FIT = False

MODE = INTERPOLATE
# MODE = FIT

if MODE == INTERPOLATE:
    from ellipse_interpolation import fit_ellipse, preprocessing
else:
    from ellipse_fitting import fit_ellipse, preprocessing


def args(img, name):
    for i in range(1, 256):
        yield(img, i, name)


if __name__ == "__main__":
    dir = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q4-upravene-spravne/all/"

    with open(dir + "list2.txt") as f:
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
            else:
                img = preprocessing(img, size=(1024, 1024), angle=151)

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
