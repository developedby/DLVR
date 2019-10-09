#include "RadioCommunication.h"
#include <iostream>

int main()
{
	RadioCommunication radio = RadioCommunication();
	uint8_t address[] = {COORD_ADDRESS};
	radio.setAddress(address);
	while(radio.isChipConnected())
	{
		//std::cout<<"nada"<<std::endl;
		if(radio.receiveFromRadio())
		{
			std::cout <<"recebi pacote"<<std::endl;
			int size = radio.getDataSize();
			unsigned char data[size];
			radio.getData(data);
			radio.sendToRadio(data, size);
			std::cout << "mandei de volta"<<std::endl;
		}
	}
	std::cout <<"acabou" <<std::endl;
}
