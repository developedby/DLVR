import numpy as np
import cv2

path = "./img/street/"
img_in = "green"
extension = ".jpg"

img = cv2.imread(path + img_in + extension)
if img is None:
    print("File not found")
    exit()
img = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)

h_min = 60
h_max = 90
l_min = 70
l_max = 150
s_min = 30
s_max = 120

h_mask = cv2.inRange(img[:, :, 0], h_min, h_max)
l_mask = cv2.inRange(img[:, :, 1], l_min, l_max)
s_mask = cv2.inRange(img[:, :, 2], s_min, s_max)
mask = cv2.bitwise_and(h_mask, l_mask)
mask = cv2.bitwise_and(mask, s_mask)
krn = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, krn)
mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, krn)
mask = cv2.ximgproc.thinning(mask)
mask = cv2.dilate(mask, krn)

mask_img = img.copy()
mask_img[:, :, 1] = cv2.copyTo(img[:, :, 1], mask)
mask_img = cv2.cvtColor(mask_img, cv2.COLOR_HLS2BGR)
cv2.imshow('mask', mask_img)

lines = cv2.HoughLinesP(mask, 1, np.radians(1), 100,
                        minLineLength=100)
if lines is None:
    print("no lines found")
    exit()
lines = lines[:, 0]

img_lines = cv2.cvtColor(img, cv2.COLOR_HLS2BGR)
for line in lines:
    cv2.line(img_lines, (line[0], line[1]), (line[2], line[3]),
             (0, 255, 0), thickness=1, lineType=cv2.LINE_AA)
cv2.imshow('found lines', img_lines)

while (cv2.waitKey(1) & 0xFF) != 27:
    pass
cv2.destroyAllWindows()