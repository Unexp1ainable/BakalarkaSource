from scipy.interpolate import PchipInterpolator
from math import isclose, log, sqrt
from multiprocessing.synchronize import Lock
from os import mkdir
from typing import List, Tuple
import cv2 as cv
import numpy as np
import matplotlib.pyplot as plt
from scipy import ndimage
from skimage.morphology import skeletonize
from numba import jit
from ellipse_common import *


UP = True
DOWN = False

STEP_A = 2
STEP_B = 2
STEP_C = 0.1
STEP_K = 0.5

DIR_A = None
DIR_B = None
DIR_C = None
DIR_K = None

ROUGH_ITERATIONS = 10
SMOOTH_ITERATIONS = 10
VISUALIZE = False


def show(a, b, c, k):
    dimg = img.copy()
    draw_ellipse(dimg, a, b, c, k)
    cv.imshow("tmp", dimg)
    cv.waitKey(1)


def fit_a(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:

    pts = [
        (0.0, 0.4125709533691406),
        (7.5, 0.43141937255859375),
        (15.0, 0.4502677917480469),
        (22.5, 0.4607391357421875),
        (30.0, 0.47330474853515625),
        (37.75, 0.4795875549316406),
        (45.25, 0.4921531677246094),
        (52.75, 0.49843597412109375),
        (60.25, 0.5005302429199219),
        (67.75, 0.5005302429199219),
        (75.25, 0.50262451171875),
        (82.75, 0.49843597412109375),
        (90.25, 0.49005889892578125),
        (98.0, 0.4795875549316406),
        (105.5, 0.4774932861328125),
        (113.0, 0.46492767333984375),
        (120.5, 0.4544563293457031),
        (128.0, 0.4460792541503906),
        (135.5, 0.4293251037597656),
        (143.0, 0.4209480285644531),
        (150.5, 0.4083824157714844),
        (158.0, 0.3874397277832031),
        (165.75, 0.351837158203125),
        (173.25, 0.3288002014160156),
        (180.75, 0.2952919006347656),
        (188.25, 0.2534065246582031),
        (195.75, 0.19476699829101562),
        (203.25, 0.117279052734375),
        (210.75, 0.0),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]*width


def fit_b(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> float:

    pts = [
        (0.0, 0.2392120361328125),
        (7.5, 0.23725128173828125),
        (15.0, 0.23529052734375),
        (22.5, 0.23431015014648438),
        (30.0, 0.23529052734375),
        (37.75, 0.23627090454101562),
        (45.25, 0.23725128173828125),
        (52.75, 0.23823165893554688),
        (60.25, 0.24019241333007812),
        (67.75, 0.24019241333007812),
        (75.25, 0.23823165893554688),
        (82.75, 0.23725128173828125),
        (90.25, 0.23431015014648438),
        (98.0, 0.23234939575195312),
        (105.5, 0.2313690185546875),
        (113.0, 0.22842788696289062),
        (120.5, 0.22940826416015625),
        (128.0, 0.22842788696289062),
        (135.5, 0.22548675537109375),
        (143.0, 0.22254562377929688),
        (150.5, 0.21274185180664062),
        (158.0, 0.1999969482421875),
        (165.75, 0.17940902709960938),
        (173.25, 0.15587997436523438),
        (180.75, 0.133331298828125),
        (188.25, 0.11078262329101562),
        (195.75, 0.08039093017578125),
        (203.25, 0.04117584228515625),
        (210.75, 0.0),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]*height


# @jit(nopython=True)
def fit_c(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:

    return 2.1

    lastRank = rank_ellipse(a, b, c, h, k, points, height, width)
    # determine direction
    currRank = rank_ellipse(a, b, c + STEP_C, h, k, points, height, width)
    if lastRank < currRank:
        DIR_C = DOWN
    else:
        DIR_C = UP
        c += STEP_C

    while True:
        if DIR_C == UP:
            currRank = rank_ellipse(a, b, c + STEP_C, h, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                c += STEP_C
                if c > 7:
                    break
        else:
            currRank = rank_ellipse(a, b, c - STEP_C, h, k, points, height, width)
            if lastRank < currRank:
                break
            else:
                if c - STEP_C <= 1.5:
                    break
                c -= STEP_C

    # if True:
    #     show(a, b, c, k)
    return c


def fit_k(a: float, b: float, c: float, h: float, k: float, points: List[Tuple[int, int]],
          height, width, val) -> Tuple[float, float, float, float]:
    pts = [
        (0.0, 0.0),
        (7.5, 0.0),
        (15.0, -0.0017337799072265625),
        (22.5, 0.07975387573242188),
        (30.0, 0.14910507202148438),
        (37.75, 0.1803131103515625),
        (45.25, 0.2080535888671875),
        (52.75, 0.2305927276611328),
        (60.25, 0.2531318664550781),
        (67.75, 0.27740478515625),
        (75.25, 0.29821014404296875),
        (82.75, 0.3224830627441406),
        (90.25, 0.34848976135253906),
        (98.0, 0.3744964599609375),
        (105.5, 0.3987693786621094),
        (113.0, 0.42824363708496094),
        (120.5, 0.4542503356933594),
        (128.0, 0.4767894744873047),
        (135.5, 0.5114650726318359),
        (143.0, 0.5357379913330078),
        (150.5, 0.5634784698486328),
        (158.0, 0.5894851684570312),
        (165.75, 0.6276283264160156),
        (173.25, 0.6588363647460938),
        (180.75, 0.7004470825195312),
        (188.25, 0.7420578002929688),
        (195.75, 0.7888698577880859),
        (203.25, 0.8460845947265625),
        (210.75, 0.8876953125),
        (218.25, 0.0),
        (226.0, 0.0),
        (233.5, 0.0),
        (241.0, 0.0),
        (248.5, 0.0)]
    cs = PchipInterpolator(list(np.arange(0, 256, 256/len(pts))), pts)
    return cs(val)[1]*height


def fit_ellipse(img: np.ndarray, value: int = None, name: str = "") -> Tuple[float, float, float, float, float]:
    if type(img) == tuple:  # multiprocessing arguments
        value = img[1]
        name = img[2]
        img = img[0]

    print("Started with value ", value)
    if value is not None:
        img = processing(img, value)

    bounds = get_bounds(img)
    if bounds[0] >= bounds[2] or bounds[1] >= bounds[3]:
        print("Error, no pixels of interest. " + str(value))
        return (0., 0., 0., 0., 0.)

    points = np.argwhere(img)
    height = img.shape[0]
    width = img.shape[1]

    a = round((bounds[2]-bounds[0])*0.5)
    b = a/2
    c = 2
    k = -bounds[1]

    a = fit_a(a, b, c, 0, k, points, height, width, value)
    b = fit_b(a, b, c, 0, k, points, height, width, value)
    k = fit_k(a, b, c, 0, k, points, height, width, value)
    c = fit_c(a, b, c, 0, k, points, height, width, value)

    print("convergence reached " + str(value))
    draw_ellipse(img, a, b, c, 0, k)

    outdir = "output/"
    if name:
        outdir = "output/" + name[:-4] + "/"

    cv.imwrite(outdir + "img" + str(value) + ".png", img)

    try:
        with open(outdir + "output.csv", "a") as file:
            file.write(str(value) + ";" + str(a) + ";" + str(b) + ";" + str(c) + ";" + str(k) + "\n")
    except:
        print("Fitted for " + str(value) + ": a=" + str(a) + ", b=" + str(b) + ", c=" + str(c) + ", k=" + str(k) + "\n")
    return (a, b, c, 0, k)


if __name__ == "__main__":
    PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q1-upravene/all/5kV_105_1_u.png"
    VALUE = 80
    VISUALIZE = True
    ANGLE = -11

    global img
    img = cv.imread(PATH, cv.IMREAD_GRAYSCALE)
    img = preprocessing(img, ANGLE)
    imgcopy = img.copy()
    img: np.ndarray = processing(img, VALUE)  # prepare visualizing image

    # shap = img.shape
    # img.resize((shap[0]+150, shap[1]), refcheck=False)
    res = fit_ellipse(imgcopy, VALUE, "ba.png")

    # prepare final image

    # draw final image
    draw_ellipse(img, *res)
    cv.imshow("fitted", img)
    cv.waitKey(0)
    cv.destroyAllWindows()
