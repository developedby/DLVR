import numpy as np
import cv2

path = "./img/street/"
img_in = "comprido"
extension = ".png"

img = cv2.imread(path + img_in + extension)
if img is None:
    print("File not found")
    exit()
cv2.imshow('original', img)
h, w, ch = img.shape
print(img.shape)
cv2.namedWindow('params')
cv2.createTrackbar('pt1 x', 'params', 0, w, lambda _: None)
cv2.createTrackbar('pt1 y', 'params', 0, h, lambda _: None)
cv2.createTrackbar('pt2 x', 'params', 0, w, lambda _: None)
cv2.createTrackbar('pt2 y', 'params', 0, h, lambda _: None)
cv2.createTrackbar('pt3 x', 'params', 0, w, lambda _: None)
cv2.createTrackbar('pt3 y', 'params', 0, h, lambda _: None)
cv2.createTrackbar('pt4 x', 'params', 0, w, lambda _: None)
cv2.createTrackbar('pt4 y', 'params', 0, h, lambda _: None)

while (cv2.waitKey(1) & 0xFF) != 27:
    pt1_x = cv2.getTrackbarPos('pt1 x','params')
    pt1_y = cv2.getTrackbarPos('pt1 y','params')
    pt2_x = cv2.getTrackbarPos('pt2 x','params')
    pt2_y = cv2.getTrackbarPos('pt2 y','params')
    pt3_x = cv2.getTrackbarPos('pt3 x','params')
    pt3_y = cv2.getTrackbarPos('pt3 y','params')
    pt4_x = cv2.getTrackbarPos('pt4 x','params')
    pt4_y = cv2.getTrackbarPos('pt4 y','params')
    #src = np.float32([[305, 260], [485, 260], [0, 450], [820, 450]])
    #dst = np.float32([[0, 0], [300, 0], [0, 300], [300, 300]])
    src = np.float32([(pt1_x, pt1_y), (pt2_x, pt2_y), (pt3_x, pt3_y), (pt4_x, pt4_y)])
    dst = np.float32([[0, 0], [w, 0], [0, h], [w, h]])
    persp_mat = cv2.getPerspectiveTransform(src, dst)
    #print(persp_mat)
    #persp_mat = np.float32([[1, 0, 0], [0, 1, 0], [0, 0, 1]])
    warped = cv2.warpPerspective(img, persp_mat, (w, h))
    
    cv2.imshow('warped', warped)
    pass
cv2.destroyAllWindows()
