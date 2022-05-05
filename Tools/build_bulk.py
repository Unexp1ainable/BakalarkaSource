import spline_builder as sp
from argparse import Namespace
import os

if __name__ == "__main__":
    for mode in ["c", "k"]:
        for dir in os.listdir("output"):
            print(dir)
            with open("tmp.txt", "a", encoding="utf-8") as f:
                print(dir, file=f)
            path = f"output/{dir}/output.csv"
            args = Namespace(path=path, mode=mode)
            sp.run(args, dir+"-"+mode)
            with open("tmp.txt", "a", encoding="utf-8") as f:
                print("", file=f)
