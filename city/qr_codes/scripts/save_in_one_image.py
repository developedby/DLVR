import numpy
import cv2

imgs = []
for i in range(32):
    imgs.append(cv2.imread(f'aruco{i}.png'))
h, w, c = imgs[0].shape
s = 100

result = numpy.ndarray(((h+s)*6, (w+s)*6, c))
result += 255
for i in range(32):
    #result[((i//6)*(h+s)), (i*(w+s))%6] = imgs[i]
    result[((i//6)*(h+s)):((i//6)*(h+s))+h, ((i%6)*(w+s)) : ((i%6)*(w+s)) + h] = imgs[i]
cv2.imwrite("result.png", result)
