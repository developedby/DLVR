#include <iostream>
#include <pigpio.h>
#include "radio_communication.hpp"
#include "constants.hpp"

int main()
{
	if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
	RadioCommunication radio = RadioCommunication();
	radio.setAddress(constants::coord_address);
	
	while(radio.isChipConnected())
	{
		/*std::vector<uint16_t> qr_codes_read;
		qr_codes_read.push_back(1);
		qr_codes_read.push_back(2);
		qr_codes_read.push_back(3);
		float ultrassound_reading;
		uint8_t other_sensors_reading;
		ultrassound_reading = 2.345678;
		other_sensors_reading = NO_SENSOR_READ;
		Movement movement;
		status::Status status;
		movement.read = false;
		status = status::MOVING_FORWARD;
		radio.sendToRadio(SendedMessage(qr_codes_read, ultrassound_reading, other_sensors_reading, movement, status));*/
		if(radio.receiveFromRadio())
		{
			std::cout <<"recebi pacote"<<std::endl;
			ReceivedMessage received = radio.getData();
			std::cout << "path: ";
			//for(int i=0; i<int(received.path.size()); i++)
			for(auto i:received.path)
			{
				std::cout<<int(i) <<" ";
			}
			std::cout << std::endl;
			std::cout <<"qr_code: "<<received.qr_code<<std::endl;
			std::cout <<"command: "<<received.command<<std::endl;
			std::cout <<"sensor: "<<received.sensor_to_read<<std::endl;
			std::cout <<"required_Status: "<<received.required_status<<std::endl;
			std::vector<uint16_t> qr_codes_read;
			qr_codes_read.push_back(1);
			qr_codes_read.push_back(2);
			qr_codes_read.push_back(3);
			float ultrassound_reading;
			uint8_t other_sensors_reading;
			if(received.sensor_to_read == ULTRASOUND)
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
			MovementInfo movement;
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
			radio.sendToRadio(SentMessage(qr_codes_read, ultrassound_reading, other_sensors_reading, movement, status));
		}
	}
	std::cout <<"radio desconectado" <<std::endl;
	return 0;
}
