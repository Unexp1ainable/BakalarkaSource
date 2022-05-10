import numpy as np
import re
import matplotlib.pyplot as plt

params_arr = ["a", "b", "k"]
params_scalar = ["c", "roil", "roih"]
wds = ["10", "12", "14", "16", "18", "20"]
energies = ["5", "10", "15", "20", "25", "30"]


def loadA(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()[:-4]

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadB(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()[:-4]

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadC(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]+kV.*", line):
                name = line.strip()[:-4]

            elif re.match(r"[0-9]", line[0]):
                result[name] = float(line.strip())

    return result


def loadK(path: str):
    with open(path) as f:
        name = ""
        result = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()[:-4]

            elif line[0] == "[":
                result[name] = list(map(float, line.strip()[1:-1].split(",")))

    return result


def loadROI(path: str):
    with open(path) as f:
        name = ""
        result1 = {}
        result2 = {}
        for line in f:
            if re.match(r"[0-9]", line[0]):
                name = line.strip()[:-4]

            elif line[:4] == "ROI:":
                result1[name], result2[name] = list(map(int, line[4:].strip().split(" - ")))

    return result1, result2


def loadData(dir):
    a = loadA(dir+"results_a2.txt")
    b = loadB(dir+"results_b.txt")
    c = loadC(dir+"results_c.txt")
    k = loadK(dir+"results_k.txt")
    roil, roih = loadROI(dir+"results_roi_compensated_5.txt")

    # array data
    kv = {}
    mm = {}

    for p in params_arr:
        kv[p] = {}
        mm[p] = {}

    for grp, name in zip((a, b, k), params_arr):
        for key in grp:
            m = re.match(r"^[0-9]+kV", key)
            if m is not None:
                m = m.group()[:-2]
                try:
                    kv[name][m]
                except:
                    kv[name][m] = []

                kv[name][m].append((key, grp[key]))

            m = re.search(r"[0-9]+mm", key)
            if m is not None:
                m = m.group()[:-2]
                try:
                    mm[name][m]
                except:
                    mm[name][m] = []

                mm[name][m].append((key, grp[key]))

    # scalar data
    kv_s = {}
    mm_s = {}

    for p in params_scalar:
        kv_s[p] = {}
        for e in energies:
            kv_s[p][e] = []

        mm_s[p] = {}
        for wd in wds:
            mm_s[p][wd] = []

    for grp, name in zip((c, roil, roih), params_scalar):
        for key in grp:
            en = re.match(r"^[0-9]+kV", key).group()[:-2]
            wd = re.search(r"[0-9]+mm", key).group()[:-2]
            kv_s[name][en].append((int(wd), grp[key]))
            mm_s[name][wd].append((int(en), grp[key]))

    return kv, mm, kv_s, mm_s


if __name__ == "__main__":
    REL_DIR = "app/BakalarkaTake2/relations/"
    kv, mm, kv_s, mm_s = loadData(REL_DIR)
    # plt.title("test")
    # grp = kv_s["roil"]
    # for par in grp:
    #     a = list(zip(*grp[par]))
    #     plt.plot(*a, label=par)
    # plt.legend()
    # plt.show()

    # exit(0)
    for param in ["roil", "roih"]:
        for dist in wds:
            name = param+"-wd-"+dist+"mm"
            plt.title(name)
            plt.xlabel("Beam energy")
            plt.ylabel("Pixel value")
            plt.plot(*zip(*mm_s[param][dist]), label=dist+"mm")
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()

        for e in energies:
            name = param+"-e-"+e+"kV"
            plt.title(name)
            plt.xlabel("Working distance")
            plt.ylabel("Pixel value")
            plt.plot(*zip(*kv_s[param][e]), label=e+"kV")
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()

    for param in ["c"]:
        for dist in wds:
            name = param+"-wd-"+dist+"mm"
            plt.title(name)
            plt.xlabel("Beam energy")
            plt.ylabel("Value")
            plt.plot(*zip(*mm_s[param][dist]), label=dist+"mm")
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()

        for e in energies:
            name = param+"-e-"+e+"kV"
            plt.title(name)
            plt.xlabel("Working distance")
            plt.ylabel("Value")
            plt.plot(*zip(*kv_s[param][e]), label=e+"kV")
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()

    for param in ["roil", "roih"]:
        name = param+"-wd-all"
        plt.title(name)
        plt.xlabel("Beam energy")
        plt.ylabel("Pixel value")
        for dist in wds:
            plt.plot(*zip(*mm_s[param][dist]), label=dist+"mm")
        plt.legend()
        plt.savefig("graphs/" + name + ".png")
        plt.clf()

        name = param+"-e-all"
        plt.title(name)
        plt.xlabel("Working distance")
        plt.ylabel("Pixel value")
        for e in energies:
            plt.plot(*zip(*kv_s[param][e]), label=e+"kV")
        plt.legend()
        plt.savefig("graphs/" + name + ".png")
        plt.clf()

    for param in ["c"]:
        name = param+"-wd-all"
        plt.title(name)
        plt.xlabel("Beam energy")
        plt.ylabel("Value")
        for dist in wds:
            plt.plot(*zip(*mm_s[param][dist]), label=dist+"mm")
        plt.legend()
        plt.savefig("graphs/" + name + ".png")
        plt.clf()

        name = param+"-e-all"
        plt.title(name)
        plt.xlabel("Working distance")
        plt.ylabel("Value")
        for e in energies:
            plt.plot(*zip(*kv_s[param][e]), label=e+"kV")
        plt.legend()
        plt.savefig("graphs/" + name + ".png")
        plt.clf()

    # for param in params_scalar:
    #     name = param+"-wd_all"
    #     for dist in wds:
    #         plt.title(name)

    #         plt.plot(*zip(*mm_s[param][dist]), label=dist+"mm")
    #     plt.legend()
    #     plt.savefig("graphs/" + name + ".png")
    #     plt.clf()

    # for param in params_scalar:
    #     name = param+"kV_all"
    #     for e in energies:
    #         plt.title(name)
    #         plt.plot(*zip(*kv_s[param][e]), label=e+"kV")

    #     plt.legend()
    #     plt.savefig("graphs/" + name + ".png")
    #     plt.clf()

    for param in params_arr:
        for dist in wds:
            name = param+"-wd-"+dist+"mm"
            plt.title(name)
            plt.xlabel("PCHIP point number")
            plt.ylabel("Normalised value")
            for par in mm[param][dist]:
                plt.plot(par[1], label=par[0])
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()

        for e in energies:
            name = param+"-e-"+e+"kv"
            plt.title(name)
            plt.xlabel("PCHIP point number")
            plt.ylabel("Normalised value")
            for par in kv[param][e]:
                plt.plot(par[1], label=par[0])
            plt.legend()
            plt.savefig("graphs/" + name + ".png")
            plt.clf()
