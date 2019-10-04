import numpy as np
import cv2

path = "./imgs/teste_fitas/"
img_in = "teste_fitas"
extension = ".png"

img = cv2.imread(path + img_in + extension)
img = cv2.cvtColor(img, cv2.COLOR_BGR2HLS)

img_hue = img.copy()
img_hue[:, :, 1] = 128
img_hue[:, :, 2] = 255
img_hue = cv2.cvtColor(img_hue, cv2.COLOR_HSV2BGR)
cv2.imwrite(path + img_in + " hue" + extension, img_hue)
cv2.imwrite(path + img_in + " light" + extension, img[:, :, 1])
cv2.imwrite(path + img_in + " sat" + extension, img[:, :, 2])

red_h_mask = cv2.inRange(img[:, :, 0], 170, 180)
red_l_mask = cv2.inRange(img[:, :, 1], 100, 160)
red_s_mask = cv2.inRange(img[:, :, 2], 90, 200)
red_mask = cv2.bitwise_and(red_h_mask, red_l_mask)
red_mask = cv2.bitwise_and(red_mask, red_s_mask)
red_img = img.copy()
red_img[:, :, 1] = cv2.copyTo(img[:, :, 1], red_mask)
red_img = cv2.cvtColor(red_img, cv2.COLOR_HLS2BGR)
cv2.imwrite(path + img_in + " red_mask_h" + extension, red_h_mask)
cv2.imwrite(path + img_in + " red_mask_l" + extension, red_l_mask)
cv2.imwrite(path + img_in + " red_mask_s" + extension, red_s_mask)
cv2.imwrite(path + img_in + " red_mask" + extension, red_mask)
cv2.imwrite(path + img_in + " red" + extension, red_img)
