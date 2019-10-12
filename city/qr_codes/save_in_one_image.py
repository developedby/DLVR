import numpy
import cv2

imgs = []
for i in range(32):
    imgs.push(cv2.imread(f'aruco{i}.png'))
h, w, c = imgs[0].shape
s = 10

result = numpy.zeros((h+s)*6, (w+s)*6, c)
result += 255
for i in range(32):
    result[((i//6)*(h+s)):((i//6)*(h+s))+h, (i*(w+s))%6 : ((i*(w+s))%6) + w] = imgs[i]
