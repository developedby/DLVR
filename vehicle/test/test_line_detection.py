import numpy as np
import cv2

def draw_lines(img, lines, color=(0, 255, 0), thickness=1):
    """Desenha na imagem uma sequência de linhas no formato (rho, theta)"""
    h = img.shape[0]
    for line in lines:
        rho = line[0]
        theta = line[1]
        a = np.cos(theta)
        b = np.sin(theta)
        x0 = a * rho
        y0 = b * rho
        pt1 = (int(x0 + h*(-b)), int(y0 + h*(a)))
        pt2 = (int(x0 - h*(-b)), int(y0 - h*(a)))
        cv2.line(img, pt1, pt2, color, thickness, cv2.LINE_AA)
    return img

path = "./img/street/"
img_in = "cruzamento"
extension = ".png"

img = cv2.imread(path + img_in + extension)
if img is None:
    print("File not found")
    exit()
img = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)

h_min = 100
h_max = 120
l_min = 70
l_max = 200
s_min = 30
s_max = 170

mask = cv2.inRange(img, (h_min, l_min, s_min), (h_max, l_max, s_max))
krn = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
#mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, krn)
mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, krn)

mask_img = img.copy()
mask_img[:, :, 1] = cv2.copyTo(img[:, :, 1], mask)
mask_img = cv2.cvtColor(mask_img, cv2.COLOR_HLS2BGR)
cv2.imshow('mask', mask_img)

e1 = cv2.getTickCount()
lines = cv2.HoughLinesP(mask, 1, np.radians(5), 100,
                        minLineLength=20, maxLineGap=20)
e2 = cv2.getTickCount()
print((e2 - e1)/cv2.getTickFrequency())


if lines is None:
    print("no lines found")
    exit()
lines = lines[:, 0]

img_lines = cv2.cvtColor(img, cv2.COLOR_HLS2BGR)
for line in lines:
    cv2.line(img_lines, (line[0], line[1]), (line[2], line[3]),
             (0, 255, 0), thickness=1, lineType=cv2.LINE_AA)
cv2.imshow('houghlinesp', img_lines)

while (cv2.waitKey(1) & 0xFF) != 27:
    pass
cv2.destroyAllWindows()