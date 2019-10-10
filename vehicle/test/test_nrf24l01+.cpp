#include <iostream>
#include <pigpio.h>
#include "RadioCommunication.h"

int main()
{
	if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
	RadioCommunication radio = RadioCommunication();
	uint8_t address[] = {COORD_ADDRESS};
	radio.setAddress(address);
	
	while(radio.isChipConnected())
	{
		//std::cout<<"nada"<<std::endl;
		if(radio.receiveFromRadio())
		{
			std::cout <<"recebi pacote"<<std::endl;
			ReceivedMessage received= radio.getData();
			for(auto i : received.path)
			{
				std::cout << received.path[i] <<" ";
			}
			std::cout << std::endl;
			std::cout <<received.qr_code<<std::endl;
			std::cout <<received.command<<std::endl;
			std::cout <<received.sensor_to_read<<std::endl;
			std::cout <<received.required_status<<std::endl;
			std::vector<uint16_t> qr_codes_read;
			qr_codes_read.push_back(1);
			qr_codes_read.push_back(2);
			qr_codes_read.push_back(3);
			float ultrassound_reading;
			uint8_t other_sensors_reading;
			if(received.sensor_to_read == ULTASOUND)
			{
				ultrassound_reading = 2.345678;
				other_sensors_reading = NO_SENSOR_READ;
			}
			else if(received.sensor_to_read == NO_SENSORS)
			{
				ultrassound_reading = 0;
				other_sensors_reading = NO_SENSOR_READ;
			}
			else
			{
				ultrassound_reading = 0;
				other_sensors_reading = 1;
			}
			Movement movement;
			status::Status status;
			if(received.required_status == MOVEMENT)
			{
				movement.read = true;
				movement.amplitude = 9.876;
				movement.curvature = 6.789;
			}
			else if(received.required_status == VEHICLE_STATUS)
			{
				movement.read = false;
				status = status::MOVING_FORWARD;
			}
			else if(received.required_status == NO_STATUS)
			{
				movement.read = false;
				status = status::NO_STATUS;
			}
			radio.sendToRadio(SendedMessage(qr_codes_read, ultrassound_reading, other_sensors_reading, movement, status));
			std::cout << "mandei de volta"<<std::endl;
		}
	}
	std::cout <<"acabou" <<std::endl;
	return 0;
}