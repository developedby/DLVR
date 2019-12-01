dummy:
	echo "Please choose one of the tests"

# Remove os .o de ./build
clean:
	rm ./build/*.o

# Limpa a pasta ./build	
clean_all:
	rm ./build/*

# Teste encoder
encoder: ../src/encoder.cpp test_encoder.cpp
	gcc -Wall -o ./build/test_encoder ../src/encoder.cpp test_encoder.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lraspicam -lrf24-bcm -I../include

# Teste roda
wheel: ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp test_wheel.cpp
	gcc -Wall -o ./build/test_wheel ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp test_wheel.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -I../include

# Teste radio
radio_example: test_nrf24l01_example.cpp
	gcc -Wall -o ./build/test_nrf24l01_example test_nrf24l01_example.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm -I../include

# Teste detector de item
item_detector: ../src/item_detector.cpp test_item_detector.cpp
	gcc -Wall -o ./build/test_item_detector ../src/item_detector.cpp test_item_detector.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -I../include

# Teste trava
lock: ../src/servo.cpp ../src/lock.cpp test_lock.cpp
	gcc -Wall -o ./build/test_lock ../src/servo.cpp ../src/lock.cpp test_lock.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -I../include

# Teste camera opencv
camera_opencv:
	gcc -Wall -o ./build/test_camera_opencv test_camera_opencv.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lraspicam -lraspicam_cv \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

# Teste controle
control: ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ./movement.cpp
	gcc -Wall -o ../test/build/test_control ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ./movement.cpp test_control.cpp ./pid/pid.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -I../include -I./ -I./pid

# Teste pid tunning
pid_tunning: ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ./pid_tunning.cpp ./pid/pid.cpp
	gcc -Wall -o ./pid_tunning ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ./pid_tunning.cpp ./pid/pid.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -I../include -I./ -I./pid

#Teste radio novo
radio: test_nrf24l01+.cpp ../src/radio.cpp ../src/radio_communication.cpp
	gcc -Wall -o ./build/test_nrf24l01 test_nrf24l01+.cpp ../src/radio.cpp ../src/radio_communication.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -lrf24-bcm -I../include

#Teste visão
vision: ./build/test_vision.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/ultrasound.o
	gcc -Wall -o ./build/test_vision ./build/test_vision.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/ultrasound.o \
		-lstdc++ -std=c++17 \
		-lpigpio -lrt -lm -lpthread -lrf24-bcm -lraspicam -lraspicam_cv \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_aruco -lopencv_videoio \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

# Teste acha linhas
lines: test_line_detection.cpp
	gcc -Wall -o ./build/test_line_detection test_line_detection.cpp \
		-lstdc++ -std=c++17 \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

#Teste vehicle
vehicle: ./build/vehicle_run.o ./build/movement.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o ./build/pidz.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/radio.o ./build/radio_communication.o ./build/lock.o ./build/servo.o ./build/item_detector.o ./build/intelligence.o ./build/vehicle.o ./build/delivery_box.o ./build/ultrasound.o ./build/street_follower.o
	gcc -Wall -o ./build/vehicle_run ./build/vehicle_run.o ./build/movement.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o ./build/pidz.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/radio.o ./build/radio_communication.o ./build/lock.o ./build/item_detector.o ./build/intelligence.o ./build/vehicle.o ./build/delivery_box.o ./build/servo.o ./build/ultrasound.o ./build/street_follower.o\
		-lstdc++ -std=c++17 \
		-lpigpio -lrt -lm -lpthread -lrf24-bcm -lraspicam -lraspicam_cv\
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_aruco -lopencv_videoio\
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

#teste seguir rua
follow_road:  ./build/test_follow_road.o ./build/movement.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o ./build/pidz.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/ultrasound.o
	gcc -Wall -o ./build/test_follow_road ./build/test_follow_road.o ./build/movement.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o ./build/pidz.o ./build/vision.o ./build/streets.o ./build/geometry.o ./build/ultrasound.o \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lraspicam -lraspicam_cv -lopencv_aruco -lopencv_videoio\
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
#Turn test
turn: ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ../src/movement.cpp
	gcc -Wall -o ./build/test_turn ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp ../src/movement.cpp ../src/pidz.cpp ./test_turn.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 \
		-I../include -I./

#Test Alefe Control
test_control: ./build/move_straigth.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o 
	gcc -Wall -o ./build/test_control ./build/move_straigth.o ./build/dc_motor.o ./build/encoder.o ./build/wheel.o \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

# Teste semaforo
traffic_light: test_camera_traffic_light.cpp
	gcc -Wall -o ./build/test_camera_traffic_light test_camera_traffic_light.cpp \
		-lstdc++ -std=c++17 \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4


# Compilation commands:
./build/test_follow_road.o: test_follow_road.cpp
	gcc -c -Wall -o ./build/test_follow_road.o test_follow_road.cpp \
		-lstdc++ -std=c++17 \
		-lpigpio -lrt -lm -lpthread -lrf24-bcm -lraspicam -lraspicam_cv \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_aruco\
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/test_vision.o: test_vision.cpp
	gcc -c -Wall -o ./build/test_vision.o test_vision.cpp \
		-lstdc++ -std=c++17 \
		-lpigpio -lrt -lm -lpthread -lrf24-bcm -lraspicam -lraspicam_cv \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_aruco -lopencv_videoio \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
		
./build/vehicle_run.o: vehicle_run.cpp
	gcc -c -Wall -o ./build/vehicle_run.o vehicle_run.cpp \
		-lstdc++ -std=c++17 \
		-lpigpio -lrt -lm -lpthread -lrf24-bcm -lraspicam -lraspicam_cv \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_aruco \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/move_straigth.o: move_straight.cpp
	gcc -c -Wall -o ./build/move_straigth.o move_straight.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/movement.o: ../src/movement.cpp
	gcc -c -Wall -o ./build/movement.o ../src/movement.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include 
		 

./build/dc_motor.o: ../src/dc_motor.cpp
	gcc -c -Wall -o ./build/dc_motor.o ../src/dc_motor.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include 
    
./build/encoder.o: ../src/encoder.cpp
	gcc -c -Wall -o ./build/encoder.o ../src/encoder.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/wheel.o: ../src/wheel.cpp
	gcc -c -Wall -o ./build/wheel.o ../src/wheel.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/pidz.o: ../src/pidz.cpp
	gcc -c -Wall -o ./build/pidz.o ../src/pidz.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/vision.o: ../src/vision.cpp
	gcc -c -Wall -o ./build/vision.o ../src/vision.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lraspicam -lraspicam_cv -lopencv_aruco -lopencv_videoio \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/streets.o: ../src/streets.cpp
	gcc -c -Wall -o ./build/streets.o ../src/streets.cpp \
		-lstdc++ -std=c++17 -lm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/geometry.o: ../src/geometry.cpp
	gcc -c -Wall -o ./build/geometry.o ../src/geometry.cpp \
		-lm -lstdc++ -std=c++17 -lopencv_core \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/radio.o: ../src/radio.cpp
	gcc -c -Wall -o ./build/radio.o ../src/radio.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/radio_communication.o: ../src/radio_communication.cpp
	gcc -c -Wall -o ./build/radio_communication.o ../src/radio_communication.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/servo.o: ../src/servo.cpp
	gcc -c -Wall -o ./build/servo.o ../src/servo.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/lock.o: ../src/lock.cpp
	gcc -c -Wall -o ./build/lock.o ../src/lock.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/item_detector.o: ../src/item_detector.cpp
	gcc -c -Wall -o ./build/item_detector.o ../src/item_detector.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/delivery_box.o: ../src/delivery_box.cpp
	gcc -c -Wall -o ./build/delivery_box.o ../src/delivery_box.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/intelligence.o: ../src/intelligence.cpp
	gcc -c -Wall -o ./build/intelligence.o ../src/intelligence.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lraspicam -lraspicam_cv -lopencv_aruco \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
		
./build/vehicle.o: ../src/vehicle.cpp
	gcc -c -Wall -o ./build/vehicle.o ../src/vehicle.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lraspicam -lraspicam_cv -lopencv_aruco\
		-I../include -I/usr/local/include -I/usr/local/include/opencv4

./build/ultrasound.o: ../src/ultrasound.cpp
	gcc -c -Wall -o ./build/ultrasound.o ../src/ultrasound.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-I../include

./build/street_follower.o: ../src/street_follower.cpp
	gcc -c -Wall -o ./build/street_follower.o ../src/street_follower.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -std=c++17 -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lraspicam -lraspicam_cv -lopencv_aruco \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
