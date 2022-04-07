from ellipse_guessing import raster_ellipse
from matplotlib import pyplot as plt
import numpy as np

if __name__ == "__main__":
    h = 200
    w = 200

    res = np.zeros((256, 4), np.float64)
    with open("output.csv") as file:
        for line in file:
            try:
                value, a, b, c, k = line.split(";")
                a = float(a)
                b = float(b)
                c = float(c)
                k = float(k)
                res[int(value)] = (a, b, c, k)

            except:
                pass

    # for i in range(256):
    #     if res[i].any():
    #         a, b, c, k = res[i]
    #         points = raster_ellipse(float(a), float(b), float(c), float(k), h, w)
    #         plt.xlim(0, w)
    #         plt.ylim(0, h)
    #         plt.title(str(i))
    #         plt.scatter(*zip(*points))
    #         plt.show()

    t = res.T
    plt.title("a")
    plt.plot(t[0])
    plt.figure()

    plt.title("b")
    plt.plot(t[1])
    plt.figure()

    plt.title("c")
    plt.plot(t[2])
    plt.figure()

    plt.title("k")
    plt.plot(t[3])

    plt.show()
