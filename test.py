import cv2
from old.helplib import *
from matplotlib import pyplot as plt
import numpy as np

dir = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/data/solder_balls/1/cropped/"
imgs = loadImages(dir)

img = imgs[0]

der0 = cv2.Sobel(img, cv2.CV_64F, 1, 0)
der1 = cv2.Sobel(img, cv2.CV_64F, 0, 1)
der = abs(der0+der1)
der = der/der.max() * 255
der = der.astype(np.uint8)


a = plt.subplot(1, 2, 1)
a.imshow(der)
b = plt.subplot(1, 2, 2)

b.imshow(img)
plt.show()
