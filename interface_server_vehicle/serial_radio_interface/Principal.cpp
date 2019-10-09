#include "Principal.h"

Principal::Principal()
{
    serial = new SerialCommunication(); //comunicacao Serial1
    radio = new RadioCommunication(); //envio de dados
}

void Principal::start()
{
    uint8_t address[W_ADDRESS];
    uint8_t size_serial = 0;
    uint8_t size_radio = 0;
    uint8_t attemps = 0;
	  while(radio->isChipConnected())
	  //while(true)
   {
        if(serial->receiveFromSerial())
        {
            serial->getAddress(address);
	          radio->setAddress(address);
            size_serial = serial->getDataSize();
            unsigned char data_to_radio[size_serial];
            serial->getData(data_to_radio);
            attemps = 0;
            while(!radio->sendToRadio(data_to_radio, size_serial))
            {
                attemps++;
                if(attemps > RETRIES)
                {
                    unsigned char data_to_serial[] = {START_BYTE, 0xff, START_BYTE};
                    serial->sendToSerial(data_to_serial, 3);
                    break;
                }
            }
        }
        if(radio->receiveFromRadio())
        { 
            size_radio = radio->getDataSize();
            unsigned char data_to_serial[size_radio];
            radio->getData(data_to_serial);
            serial->sendToSerial(data_to_serial, size_radio);
        }
    }
    Serial.println("fim");
}
