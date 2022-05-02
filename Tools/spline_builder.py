from sys import stderr
from matplotlib.pyplot import yscale
import numpy as np
import cv2 as cv
from scipy.interpolate import PchipInterpolator
import re
from argparse import ArgumentParser

WNAME = "Builder"
H = 512
W = 1024
RESOLUTION = (H, W, 3)

POINTS_N = 33

MODES = {"value": 0,
         "a": 1,
         "b": 2,
         "c": 3,
         "k": 4}
BG_COLOR = 30
BORDER = 25
RADIUS = 5

POINTS_N -= 1  # hehehehe
POINT_GAP = 256/POINTS_N

HLINES_N = 10
VLINES_N = 16

# region of interest borders
ROI0 = 0
ROI1 = 1
roi_low = 0
roi_high = W


selected = None
roi_selected = None
nodes = []


def parse_args():
    parser = ArgumentParser()
    parser.add_argument("path", help="Path to .csv file with data.")
    parser.add_argument("mode", choices=["value", "a", "b", "c", "k"], help="Which parameter to interpolate.")
    return parser.parse_args()


def prepare_canvas() -> np.ndarray:
    """Prepare canvas with the backgorund, but without the borders

    Returns:
        np.ndarray: Canvas core
    """
    # paint the canvas with BG_COLOR
    img = np.full(RESOLUTION, BG_COLOR, np.uint8)

    # paint the vertical lines
    for i in np.arange(0, 256, 256/VLINES_N):
        cv.line(img, (round(i*WSCALE), 0), (round(i*WSCALE), H), (70, 70, 70), 1, )

    # paint the horizontal lines
    t = H/HLINES_N
    for i in range(HLINES_N):
        cv.line(img, (0, round(i*t)), (W, round(i*t)), (70, 70, 70), 1)
    return img


def finalize_canvas(img: np.ndarray) -> np.ndarray:
    """Create border and annotate the lines

    Args:
        img (np.ndarray): Canvas core

    Returns:
        np.ndarray: The whole canvas template
    """
    # create the canvas border
    img = cv.copyMakeBorder(img, BORDER, BORDER, BORDER, BORDER, cv.BORDER_CONSTANT,
                            value=(BG_COLOR, BG_COLOR, BG_COLOR))
    cv.line(img, (W+BORDER, BORDER), (W+BORDER, H+BORDER), (70, 70, 70), 1, )
    cv.line(img, (BORDER, H+BORDER), (W+BORDER, H+BORDER), (230, 230, 230), 1)
    cv.line(img, (BORDER, BORDER), (BORDER, H+BORDER), (230, 230, 230), 1)

    # annotate vertical lines
    for i in np.arange(0, 256+1, 256/(VLINES_N)):
        tsize = cv.getTextSize(f"{i:.1f}", cv.FONT_HERSHEY_SIMPLEX, 0.5, 1)[0]
        cv.putText(img, f"{i:.1f}", (BORDER+round(i*WSCALE-tsize[0]//2), round(H+BORDER*2)-tsize[1]//2),
                   cv.FONT_HERSHEY_SIMPLEX, 0.5, (230, 230, 230),)

    # annotate horizontal lines
    t = H/HLINES_N
    s = 1/HLINES_N
    for i in range(11):
        cv.putText(img, f"{s*i:.1f}", (0, H-round(i*t)+BORDER),
                   cv.FONT_HERSHEY_SIMPLEX, 0.3, (230, 230, 230),)

    return img


def make_template(args):
    global HSCALE, WSCALE, WSCALEI, roi_high

    if args.mode not in MODES:
        print("Column does not exist.", file=stderr)
        exit(1)

    col = MODES[args.mode]
    global data
    data = np.zeros((256+1, 1), np.float64)

    with open(args.path) as file:
        for line in file:
            try:
                tokens = line.strip().split(";")
                x = int(tokens[0])
                y = float(tokens[col])
                data[x] = y
            except:
                pass

    HSCALE = H
    WSCALE = W/256
    WSCALEI = round(WSCALE)
    roi_high = round(np.argmax(np.nonzero(data))*WSCALE)+BORDER

    final = prepare_canvas()

    for x, y in enumerate(data[:-1]):
        y = y[0]
        pt1 = (round(x*WSCALE), H-round(y*HSCALE))
        pt2 = (round((x+1)*WSCALE), H-round(data[x+1][0]*HSCALE))
        cv.line(final, pt1, pt2, (255, 100, 100), 2)

    return finalize_canvas(final)


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
    global selected, roi_selected
    if abs(-0.02*HSCALE-y) < RADIUS+2:
        if abs(roi_low-x) < RADIUS+2:
            roi_selected = ROI0
        elif abs(roi_high-x) < RADIUS+2:
            roi_selected = ROI1
        return

    for i, node in enumerate(nodes):
        if abs(node[0]-x) < RADIUS+2 and abs(node[1]-y) < RADIUS+2:
            selected = i
            return


def onMouseRelease():
    global selected, roi_selected
    selected = None
    roi_selected = None


def onMouseMotion(x, y):
    global nodes, roi_low, roi_high
    if selected is not None:
        if y < 0:
            y = 0
        if y > H:
            y = H
        s = nodes[selected]
        nodes[selected] = (s[0], y)
        update_image()
    elif roi_selected is not None:
        if roi_selected == ROI0:
            if 0 < x < roi_high-10:
                roi_low = x
        else:
            if roi_low+10 < x < W:
                roi_high = x
        initialize_nodes()
        update_image()


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


def update_image():
    img = template.copy()

    # roi handles and lines
    x = round(roi_low)+BORDER
    y = round(0.02*HSCALE)
    cv.line(img, (x, y), (x, H+BORDER), (0, 255, 0), 2)
    cv.circle(img, (x, y), 5, (0, 255, 0), 3)

    x = round(roi_high)+BORDER
    cv.circle(img, (x, y), 5, (0, 255, 0), 3)
    cv.line(img, (x, y), (x, H+BORDER), (0, 255, 0), 2)

    # curve handles
    for x, y in nodes:
        cv.circle(img, (x+BORDER, y+BORDER), RADIUS, (0, 0, 255), 2)

    # paint the spline
    spline = PchipInterpolator(*zip(*nodes))
    pts = spline(list(range(roi_low, roi_high+1, WSCALEI)))
    pt1 = (roi_low+BORDER, round(pts[0])+BORDER)
    for i, y in enumerate(pts):
        pt2 = ((roi_low+i*WSCALEI+BORDER), (round(y)+BORDER))
        cv.line(img, pt1, pt2, (150, 255, 150), 3)
        pt1 = pt2

    cv.imshow(WNAME, img)


def initialize_nodes():
    global nodes
    nodes = []
    for i in np.arange(roi_low, roi_high+1, (roi_high-roi_low)/POINTS_N):
        x = round(i)
        nodes.append((x, H-round(data[round(i/WSCALE)][0]*HSCALE)))


if __name__ == "__main__":
    global template

    args = parse_args()
    template = make_template(args)

    cv.namedWindow(WNAME)
    cv.setMouseCallback(WNAME, onMouseEvent)
    initialize_nodes()
    update_image()

    while True:
        key = cv.waitKey(10)
        if key == 27:  # esc
            break
        elif key == 13:  # enter
            arr = np.array(list((zip(*nodes))), np.float64)
            arr[1] = (H-arr[1]) / HSCALE
            print(list(zip(np.arange(0, 256+1, POINT_GAP), arr[1])))
            print(f"ROI: {round(roi_low/WSCALE)} - {round(roi_high//WSCALE)}")
        elif key == 108:  # l
            l = input("Insert loading string: ")
            load(l)

        # close on manual window close
        if cv.getWindowProperty(WNAME, cv.WND_PROP_VISIBLE) < 1:
            break
    cv.destroyAllWindows()
