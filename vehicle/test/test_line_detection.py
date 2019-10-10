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

h, l, s = cv2.split(img)

h_mask = cv2.inRange(h, h_min, h_max)
l_mask = cv2.inRange(l, l_min, l_max)
s_mask = cv2.inRange(s, s_min, s_max)
mask = cv2.bitwise_and(h_mask, l_mask)
mask = cv2.bitwise_and(mask, s_mask)
krn = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
#mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, krn)
mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, krn)

mask_img = img.copy()
mask_img[:, :, 1] = cv2.copyTo(img[:, :, 1], mask)
mask_img = cv2.cvtColor(mask_img, cv2.COLOR_HLS2BGR)
cv2.imshow('mask', mask_img)

e1 = cv2.getTickCount()

dx = cv2.Scharr(mask, cv2.CV_32F, 1, 0)
dy = cv2.Scharr(mask, cv2.CV_32F, 0, 1)
grad_mag = (np.abs(dx) + np.abs(dy))/2
grad_orient = np.degrees(np.arctan2(dy, dx)/2 + np.pi)

e2 = cv2.getTickCount()
print((e2 - e1)/cv2.getTickFrequency())

img_gradient = np.empty(img.shape, dtype=np.uint8)
img_gradient[:, :, 0] = grad_orient
img_gradient[:, :, 1] = 128
img_gradient[:, :, 2] = grad_mag
img_gradient = cv2.cvtColor(img_gradient, cv2.COLOR_HLS2BGR)
cv2.imshow('gradient orientation', img_gradient)

lines = cv2.HoughLinesP(mask, 1, np.radians(1), 80,
                        minLineLength=20, maxLineGap=10)
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