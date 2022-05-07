import numpy as np
import cv2 as cv
import matplotlib.pyplot as plt
import os


DIR = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-13 Sn bulk/proudova_zavislost/Q4/filtered/"

if __name__ == "__main__":
    for path in os.listdir(DIR):
        img = cv.imread(DIR+path, cv.IMREAD_GRAYSCALE)
        number = int(path[:-4])
        # cv.imshow("a", img)
        # cv.waitKey(0)
        # cv.destroyAllWindows()
        h = cv.calcHist([img], [0], None, [256], [0, 256])
        h[0] = 0
        # cv.normalize(h, h)
        plt.plot(h, label=path)
        # plt.figure()

    plt.legend()
    plt.figure()

    for path in os.listdir(DIR):
        img = cv.imread(DIR+path, cv.IMREAD_GRAYSCALE)
        number = int(path[:-4])
        target = 8
        im = img.astype(np.float32)
        if number/target != 1:
            im /= number/target
            im += np.random.random(im.shape)-0.5
            im += np.random.random(im.shape)-0.5
            img = np.round(im).astype(np.uint8)
        # cv.imshow("a", img)
        # cv.waitKey(0)
        # cv.destroyAllWindows()
        h = cv.calcHist([img], [0], None, [256], [0, 256])
        h[0] = 0
        # cv.normalize(h, h)
        plt.plot(h, label=path)
        # plt.figure()

    plt.legend()
    plt.show()
