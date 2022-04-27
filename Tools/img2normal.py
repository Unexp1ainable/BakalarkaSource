import cv2 as cv
import numpy as np

PATH = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/renders/nm_resized.png"

if __name__ == "__main__":
    img = cv.imread(PATH, cv.IMREAD_COLOR)
    if img is None:
        print("Unable to load image.")
        exit(1)

    img: np.ndarray

    with open("out.txt", "w") as f:
        f.write(f"{img.shape[0]} {img.shape[1]}\n")
        for row in img:
            for col in row:
                f.write(f"{-(col[2]-128)/255} {(col[1]-128)/128} {col[0]/255}\n")
