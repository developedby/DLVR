#include "SerialCommunication.h"

SerialCommunication::SerialCommunication()
{
    int i;
    for(i = 0; i<W_DATA+W_ADDRESS; i++)
    {
        received_data[i] = 0;
    }
}

bool SerialCommunication::receiveFromSerial()
{
    int i;
    if(Serial.available())
    {
        unsigned char first_byte = Serial.read();
        if(first_byte == START_BYTE)
        {
            siz = Serial.readBytesUntil(START_BYTE, received_data, W_DATA+W_ADDRESS);
            Serial.flush();
            if(received_data[siz - 1] == START_BYTE)
            {
                return true;
            }
        }
    }
    return false;
}

void SerialCommunication::getAddress(uint8_t *address)
{
    int i;
    for(i=0; i<W_ADDRESS; i++)
    {
        address[i] = received_data[i];
    }
}

void SerialCommunication::getData(unsigned char *data)
{
    int i;
    data[0] = START_BYTE;
    for(i=0; i<siz; i++)
    {
        data[i+1] = received_data[i+W_ADDRESS];
    }
}

int SerialCommunication::getDataSize()
{
    return siz + 1;
}

void SerialCommunication::sendToSerial(unsigned char *data, int _siz)
{
    Serial.write(data, _siz);
    siz = 0;  
}
