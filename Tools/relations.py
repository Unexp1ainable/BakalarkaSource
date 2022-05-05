import numpy as np
import re
import matplotlib.pyplot as plt

REL_DIR = "app/BakalarkaTake2/relations/"


def loadA(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadB(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadC(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]+kV.*", line):
                name = line.strip()

            elif re.match(r"[0-9]", line[0]):
                result[name] = float(line.strip())

    return result


def loadK(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadROI(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()

            elif line[:4] == "ROI:":
                result[name] = line[4:].strip().split(" - ")

    return result


if __name__ == "__main__":
    a = loadA(REL_DIR+"results_a.txt")
    b = loadB(REL_DIR+"results_b.txt")
    c = loadC(REL_DIR+"results_c.txt")
    k = loadK(REL_DIR+"results_k.txt")
    roi = loadROI(REL_DIR+"results_roi.txt")

    kv = {"a": {},
          "b": {},
          "c": {},
          "k": {},
          "roi": {}}

    mm = {"a": {},
          "b": {},
          "c": {},
          "k": {},
          "roi": {}}

    for grp, name in [(a, "a"), (b, "b"), (c, "c"), (k, "k"), (roi, "roi")]:
        for key in grp:
            m = re.match(r"[0-9]+", key).group()
            try:
                kv[name][m]
            except:
                kv[name][m] = []

            kv[name][m].append((key, grp[key]))

    for par in kv["a"]["25"]:
        plt.plot(par[1], label=par[0])

    plt.legend()
    plt.show()
