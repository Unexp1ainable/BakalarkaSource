from argparse import ArgumentParser
import cv2 as cv
import numpy as np
from pprint import pprint


def parse_args():
    parser = ArgumentParser(description="Mask out pixels with specified value.")
    parser.add_argument("image_path", type=str, help="Path to the image.")
    parser.add_argument("value", type=int, help="Value of to be masked out <0,255>.")
    parser.add_argument("--output", "-o", default="o.png", help="Ouptut path.")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    img = cv.imread(args.image_path, cv.IMREAD_GRAYSCALE)
    mask: np.ndarray = img == args.value
    mask = mask.astype(np.uint8)
    mask *= 255
    cv.imwrite(args.output, mask)
