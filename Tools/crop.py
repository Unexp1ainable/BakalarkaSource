from os import mkdir, walk
from PIL import Image
import re


def loadStripSize(hdrPath: str) -> int:
    file = open(hdrPath)
    for line in file:
        if re.match("ImageStripSize=[0-9]+", line):
            eq = line.find("=")
            return int(line[eq+1:])
    raise IndexError


def removeStrip(imPath: str, hdrPath: str) -> None:
    stripSize = loadStripSize(hdrPath)
    img = Image.open(imPath)
    cropped = img.crop((0, 0, img.width, img.height-stripSize))

    tpath = imPath.replace("\\", "/")
    index = tpath.rfind("/")
    try:
        mkdir(tpath[:index] + "/cropped")
    except:
        pass
    cropped.save(tpath[:index] + "/cropped/" + tpath[index+1:])


w = walk("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-13 Sn bulk", True)
for root, dirs, files in w:
    for file in files:
        if file[-4:].lower() in [".tif", ".png"]:
            # find header file
            try:
                hdr = files.index(file[:-4] + "-" + file[-3:] + ".hdr")
                try:
                    removeStrip(root + "/" + file, root + "/" + files[hdr])
                except Exception as e:
                    print(e)
            except:
                print(file[:-4] + "-" + file[-3:] + ".hdr" + " not found")
