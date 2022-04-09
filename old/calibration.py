import cv2
import numpy as np
from typing import List
from old.helplib import *

dir = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/data/cinove_koule/25_mikro/cropped/"
imgs = loadImages(dir)

avgMat = np.zeros(imgs[0].shape, np.int16)
diffSum = np.zeros(imgs[0].shape, np.int32)

for img in imgs:
    avgMat += img

avgMat = avgMat//4

for i, img in enumerate(imgs):
    wname = "img" + str(i)
    cv2.namedWindow(wname, cv2.WINDOW_KEEPRATIO or cv2.WINDOW_NORMAL)
    a = np.absolute(avgMat-img)
    diffSum += a
    # cv2.imshow(wname, a.astype(np.uint8))
    cv2.imshow(wname, img)

fimg = diffSum.astype(np.float32)
toShow = (fimg/fimg.max() * 255).astype(np.uint8)
cv2.imshow("diffSum", toShow)
cv2.waitKey(0)
cv2.destroyAllWindows()
