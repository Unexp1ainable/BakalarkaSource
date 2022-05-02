import cv2 as cv
import numpy as np
from skimage.exposure import match_histograms
import matplotlib.pyplot as plt


# Q4 is the baseline
PATH_BASELINE = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/5kV_10mm/Q4.png"


if __name__ == "__main__":
    paths = [("Q1", "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/5kV_10mm/Q1.png"),
             ("Q2", "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/5kV_10mm/Q2.png"),
             ("Q3", "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/calibration/5kV_10mm/Q3.png"), ]
    for name, path in paths:
        im_base = cv.imread(PATH_BASELINE, cv.IMREAD_GRAYSCALE)
        if im_base is None:
            print('Could not open or find the image: ', PATH_BASELINE)
            exit(0)
        im_adjust = cv.imread(path, cv.IMREAD_GRAYSCALE)
        if im_adjust is None:
            print('Could not open or find the image: ', path)
            exit(0)
        if im_base.shape != im_adjust.shape:
            print("Image sizes are different.")
            exit(0)

        h1 = cv.calcHist([im_base], [0], None, [256], [0, 256])
        h1[0] = 0

        h1 = h1/np.linalg.norm(h1)
        plt.plot(h1, label="Segment 1")
        h2 = cv.calcHist([im_adjust], [0], None, [256], [0, 256])
        h2[0] = 0
        # h2 = h2/np.linalg.norm(h2)
        # plt.plot(h2, label="Segment 2")

        offset = np.argmax(h1) - np.argmax(h2)
        gain = 1
        for i in range(10):
            gain = np.max(np.nonzero(h1)) / (np.max(np.nonzero(h2)+offset))
            offset = np.argmax(h1) - np.argmax(h2)*gain

        im_res = im_adjust.astype(np.float64)
        im_res *= gain
        im_res += offset
        im_res[im_res < 0] = 0
        im_res[im_res > 255] = 255
        im_res: np.ndarray
        # without this noise, a periodic peaks could be created due to rounding
        im_res += np.random.random(im_res.shape)-0.5
        im_adjust = im_res.round().astype(np.uint8)

        # img = np.concatenate((im_base[:im_base.shape[1]//2-1], im_adjust[im_adjust.shape[1]//2-1:im_base.shape[1]]))

        print(f"Segment {name}- Contrast: {gain}, Brightness: {offset}")

        h = cv.calcHist([im_adjust], [0], None, [256], [0, 256])
        if offset < 0:
            h[0] = 0
        else:
            h[int(offset)] = 0
            h[round(offset)] = 0
        h = h/np.linalg.norm(h)

        # cv.imshow("two", img)
        plt.plot(h, label="Segment 2")
        plt.legend()
        plt.show()
        # cv.destroyAllWindows()
