import re


FILE = "app/BakalarkaTake2/relations/results_roi_compensated.txt"
OUT_FILE = "app/BakalarkaTake2/relations/results_roi_compensated_5.txt"

TARGET = 5
currents = {"5": 10,
            "10": 2.3,
            "15": 1.4,
            "20": 1.1,
            "25": 1,
            "30": 0.75
            }


with open(FILE) as f:
    with open(OUT_FILE, "w") as of:
        curr = 1
        for line in f:
            name = re.search(r"^[0-9]+", line)
            if name:
                curr = currents[name.group()]

            if line[:4] == "ROI:":
                l, h = list(map(int, line[4:].strip().split(" - ")))
                l /= curr/TARGET
                l = round(l)
                h /= curr/TARGET
                h = round(h)
                print(f"ROI: {l} - {h}", file=of)
            else:
                of.write(line)
