from typing import List
import cv2
import numpy


def loadImages(dir: str) -> List[numpy.mat]:
    imgs = []
    with open(dir + "segments.txt") as f:
        for path in f:
            path = path.strip()
            if (path != ""):
                imgs.append(cv2.imread(dir + path))
    return imgs
