import numpy as np
import cv2

path = "./img/street/"
img_in = "semaforos"
extension = ".jpg"

img = cv2.imread(path + img_in + extension)
if img is None:
    print("File not found")
    exit()
img = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)

cv2.namedWindow('params')
cv2.createTrackbar('h min', 'params', 0, 180, lambda _: None)
cv2.createTrackbar('h max', 'params', 0, 180, lambda _: None)
cv2.createTrackbar('l min', 'params', 0, 255, lambda _: None)
cv2.createTrackbar('l max', 'params', 0, 255, lambda _: None)
cv2.createTrackbar('s min', 'params', 0, 255, lambda _: None)
cv2.createTrackbar('s max', 'params', 0, 255, lambda _: None)

img_hue = img.copy()
img_hue[:, :, 1] = 128
img_hue[:, :, 2] = 255
img_hue = cv2.cvtColor(img_hue, cv2.COLOR_HSV2BGR)
cv2.imshow('hue', img_hue);
cv2.imshow('lightness', img[:, :, 1])
cv2.imshow('saturation', img[:, :, 2])
#cv2.imwrite(path + img_in + " hue" + extension, img_hue)
#cv2.imwrite(path + img_in + " light" + extension, img[:, :, 1])
#cv2.imwrite(path + img_in + " sat" + extension, img[:, :, 2])

while True:
    h_min = cv2.getTrackbarPos('h min','params')
    h_max = cv2.getTrackbarPos('h max','params')
    l_min = cv2.getTrackbarPos('l min','params')
    l_max = cv2.getTrackbarPos('l max','params')
    s_min = cv2.getTrackbarPos('s min','params')
    s_max = cv2.getTrackbarPos('s max','params')
    h_mask = cv2.inRange(img[:, :, 0], h_min, h_max)
    l_mask = cv2.inRange(img[:, :, 1], l_min, l_max)
    s_mask = cv2.inRange(img[:, :, 2], s_min, s_max)
    mask = cv2.bitwise_and(h_mask, l_mask)
    mask = cv2.bitwise_and(mask, s_mask)
    out_img = img.copy()
    out_img[:, :, 1] = cv2.copyTo(img[:, :, 1], mask)
    out_img = cv2.cvtColor(out_img, cv2.COLOR_HLS2BGR)
    cv2.imshow('masked image', out_img)
    if (cv2.waitKey(1) & 0xFF) == 27:
        break
cv2.destroyAllWindows()
#cv2.imwrite(path + img_in + " mask_h" + extension, h_mask)
#cv2.imwrite(path + img_in + " mask_l" + extension, l_mask)
#cv2.imwrite(path + img_in + " mask_s" + extension, s_mask)
#cv2.imwrite(path + img_in + " mask" + extension, mask)
#cv2.imwrite(path + img_in + " out" + extension, out_img)
