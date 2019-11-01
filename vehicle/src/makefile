build:
	gcc -Wall -o ./build/dlvr -std=c++17\
		servo.cpp item_detector.cpp encoder.cpp dc_motor.cpp wheel.cpp ultrasound.cpp lock.cpp\
		vision.cpp street_lines.cpp geometry.cpp\ 
		-lpigpio -lrt -lm -lpthread -lstdc++ -lraspicam -lraspicam_cv -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
