import cv2

dict_ = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_250)
for i in range(32):
	img = cv2.aruco.drawMarker(dict_, i, 400, 2)
	cv2.imwrite(f'aruco{i}.png', img)
