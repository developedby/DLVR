dummy:
	echo "Please choose one of the tests"

# Teste encoder
encoder: ../src/encoder.cpp test_encoder.cpp
	gcc -Wall -o ./build/test_encoder ../src/encoder.cpp test_encoder.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -lraspicam -lrf24-bcm -I../include

# Teste roda
wheel: ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp test_wheel.cpp
	gcc -Wall -o ./build/test_wheel ../src/dc_motor.cpp ../src/encoder.cpp ../src/wheel.cpp test_wheel.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -I../include

# Teste radio
radio_example: test_nrf24l01.cpp
	gcc -Wall -o ./build/test_nrf24l01_example test_nrf24l01_example.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -lrf24-bcm -I../include

# Teste detector de item
item_detector: ../src/item_detector.cpp test_item_detector.cpp
	gcc -Wall -o ./build/test_item_detector ../src/item_detector.cpp test_item_detector.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -I../include

# Teste trava
lock: ../src/servo.cpp ../src/lock.cpp test_lock.cpp
	gcc -Wall -o ./build/test_lock ../src/servo.cpp ../src/lock.cpp test_lock.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -I../include

# Teste camera opencv
camera_opencv:
	gcc -Wall -o ./build/test_camera_opencv test_camera_opencv.cpp -lpigpio -lrt -lm -lpthread -lstdc++ -lraspicam -lraspicam_cv \
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
radio: test_nrf24l01+.cpp ../src/Radio.cpp ../src/RadioCommunication.cpp
	gcc -Wall -o ./build/test_nrf24l01 test_nrf24l01+.cpp ../src/Radio.cpp ../src/RadioCommunication.cpp \
		-lpigpio -lrt -lm -lpthread -lstdc++ -lrf24-bcm -I../include

#Teste visão
vision: test_vision.cpp ../src/vision.cpp
	gcc -Wall -o ./build/test_vision test_vision.cpp ../src/vision.cpp
		-lpigpio -lrt -lm -lpthread -lstdc++ -lrf24-bcm \
		-lopencv_core -lopencv_highgui -lopencv_imgcodecs \
		-I../include -I/usr/local/include -I/usr/local/include/opencv4
