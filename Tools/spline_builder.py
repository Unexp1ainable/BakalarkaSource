from sys import stderr
from cv2 import BORDER_CONSTANT
import numpy as np
import cv2 as cv
from scipy.interpolate import CubicSpline, PchipInterpolator
import re

WNAME = "Builder"
H = 512
W = 1024
RESOLUTION = (H, W, 3)
PATH = "output/5kV_105_1_u/output.csv"
MODE = "b"

POINTS_N = 34
POINT_GAP = 256/POINTS_N

MODES = {"value": 0,
         "a": 1,
         "b": 2,
         "c": 3,
         "k": 4}
BG_COLOR = 30
BORDER = 25
RADIUS = 5

global selected
selected = None


def prepare_canvas():
    img = np.full(RESOLUTION, BG_COLOR, np.uint8)
    for i in np.arange(0, 256, POINT_GAP):
        cv.line(img, (round(i*WSCALE), 0), (round(i*WSCALE), H), (70, 70, 70), 1, )

    t = H/6
    for i in range(6):
        cv.line(img, (0, round(i*t)), (W, round(i*t)), (70, 70, 70), 1)
    return img


def finalize_canvas(img, m):
    img = cv.copyMakeBorder(img, BORDER, BORDER, BORDER, BORDER, BORDER_CONSTANT, value=(BG_COLOR, BG_COLOR, BG_COLOR))
    cv.line(img, (BORDER, H+BORDER), (W+BORDER, H+BORDER), (230, 230, 230), 1)
    cv.line(img, (BORDER, BORDER), (BORDER, H+BORDER), (230, 230, 230), 1)
    for i in np.arange(0, 256, 15):
        tsize = cv.getTextSize(str(i), cv.FONT_HERSHEY_SIMPLEX, 0.5, 1)[0]
        cv.putText(img, str(i), (round(BORDER+i*WSCALE-tsize[0]//2), round(H+BORDER*2)-tsize[1]//2),
                   cv.FONT_HERSHEY_SIMPLEX, 0.5, (230, 230, 230),)

    t = H/6
    s = m/6
    for i in range(7):
        cv.putText(img, str(int(s*i)), (0, H-round(i*t)+BORDER),
                   cv.FONT_HERSHEY_SIMPLEX, 0.3, (230, 230, 230),)

    return img


def make_template():
    global HSCALE, WSCALE, WSCALEI

    if MODE not in MODES:
        print("Column does not exist.", file=stderr)
        exit(1)

    col = MODES[MODE]
    global data
    data = np.zeros((256, 1), np.float64)

    kcoef = 1   # I am lazy
    if MODE == "k":
        kcoef = -1

    with open(PATH) as file:
        for line in file:
            try:
                tokens = line.strip().split(";")
                x = int(tokens[0])
                y = kcoef*float(tokens[col])
                data[x] = y
            except:
                pass

    low = data.min()
    high = data.max()
    HSCALE = H/(high-low)
    WSCALE = W/256
    WSCALEI = round(WSCALE)

    final = prepare_canvas()

    for x, y in enumerate(data[:-1]):
        y = y[0]
        pt1 = (round(x*WSCALE), H-round(y*HSCALE))
        pt2 = (round((x+1)*WSCALE), H-round(data[x+1][0]*HSCALE))
        cv.line(final, pt1, pt2, (255, 100, 100), 2)

    return finalize_canvas(final, high)


def onMouseEvent(event, x, y, flags, userdata):
    x -= BORDER
    y -= BORDER
    if event == cv.EVENT_LBUTTONDOWN:
        onMousePress(x, y)
    elif event == cv.EVENT_LBUTTONUP:
        onMouseRelease()
    elif event == cv.EVENT_MOUSEMOVE:
        onMouseMotion(x, y)


def onMousePress(x, y):
    global selected
    for i, node in enumerate(nodes):
        if abs(node[0]-x) < RADIUS+2 and abs(node[1]-y) < RADIUS+2:
            selected = i
            return


def onMouseRelease():
    global selected
    selected = None


def onMouseMotion(x, y):
    global nodes
    if selected is not None:
        if y < 0:
            y = 0
        if y > H:
            y = H
        s = nodes[selected]
        nodes[selected] = (s[0], y)


def load(s):
    global nodes
    items = re.findall(r"[+-]?[0-9]+\.[0-9]*", s)
    x = []
    y = []
    switch = False
    for item in items:
        if not switch:
            x.append(float(item))
        else:
            y.append(float(item))

        switch = not switch

    if len(x) != 18 and len(y) != 18:
        print("Invalid string.")
        return

    content = zip(x, y)
    for i, ct in enumerate(content):
        nodes[i] = (round(ct[0]*WSCALE), round(H-ct[1]*HSCALE))


if __name__ == "__main__":
    global template, nodes
    template = make_template()

    nodes = []
    for i in np.arange(0, 256, POINT_GAP):
        x = round(i*WSCALE)
        nodes.append((x, H-round(data[round(i)][0]*HSCALE)))

    cv.namedWindow(WNAME)
    cv.setMouseCallback(WNAME, onMouseEvent)

    while True:
        img = template.copy()
        for x, y in nodes:
            cv.circle(img, (x+BORDER, y+BORDER), RADIUS, (0, 0, 255), 2)

        spline = PchipInterpolator(*zip(*nodes))
        pts = spline(list(range(0, round(256*WSCALE), WSCALEI)))
        pt1 = (BORDER, round(pts[0])+BORDER)
        for x in range(WSCALEI, round(256*WSCALE), WSCALEI):
            y = round(pts[round(x//WSCALE)])
            pt2 = ((x+BORDER), (y+BORDER))
            cv.line(img, pt1, pt2, (150, 255, 150), 3)
            pt1 = pt2

        cv.imshow(WNAME, img)
        key = cv.waitKey(10)
        if key == 27:  # esc
            break
        elif key == 13:  # enter
            arr = np.array(list((zip(*nodes))), np.float64)
            arr[0] /= WSCALE
            arr[1] = (H-arr[1]) / HSCALE
            print(list(zip(arr[0], arr[1])))
        elif key == 108:  # l
            l = input("Insert loading string: ")
            load(l)
    cv.destroyAllWindows()
