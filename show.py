from ellipse_guessing import raster_ellipse
from matplotlib import pyplot as plt
import numpy as np

if __name__ == "__main__":
    ress = []

    files = [
        "output/5kV_105_1_u/output.csv",
        "output/5kV_125_1_u/output.csv",
        "output/5kV_145_1_u/output.csv",
        "output/5kV_165_1_u/output.csv",
        "output/5kV_185_1_u/output.csv",
        "output/5kV_205_1_u/output.csv",
    ]
    for path in files:
        res = np.zeros((256, 4), np.float64)
        with open(path) as file:
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
        ress.append(res.T.copy())

    plt.title("a")
    for i, t in enumerate(ress):
        plt.plot(t[0], label=files[i])
    plt.legend()
    plt.figure()

    plt.title("b")
    for i, t in enumerate(ress):
        plt.plot(t[1], label=files[i])
    plt.legend()
    plt.figure()

    plt.title("c")
    for i, t in enumerate(ress):
        plt.plot(t[2], label=files[i])
    plt.legend()
    plt.figure()

    plt.title("k")
    for i, t in enumerate(ress):
        plt.plot(t[3], label=files[i])

    plt.legend()
    plt.show()
