#include "RadioCommunication.h"

RadioCommunication::RadioCommunication()
{
    int i = 0;
    radio = new Radio();
    for(i=0; i<W_DATA; i++)
    {
        received_data[i] = 0;
    }
    siz = 0;
    ack = 0;
    uint8_t address[] = {COORD_ADDRESS};
    radio->openReadingPipe(1, address);
    //radio->enableAckPayload();               // Allow optional ack payloads
    radio->enableDynamicPayloads();
    radio->startListening();
}

void RadioCommunication::setAddress(uint8_t *address)
{
    int i = 0, j=0;
    for(i=0; i<W_ADDRESS; i++)
    {
        if(address[i] != last_address[i])
        {
            radio->openWritingPipe(address);
            ack = 0;
            for(j=0; j<W_ADDRESS; j++)
            {
                last_address[j] = address[j];
            }
            break;
        }
    }
}

bool RadioCommunication::sendToRadio(const void *data, uint8_t len)
{
    unsigned long time_start = millis();
    radio->stopListening();
    radio->write(data, len);
    //radio->startListening();
//    while(!radio->available())
//    {
//        if(millis()-time_start >= TIME_OUT_RADIO_ACK)
//        {
//            return false;
//        }
//    }
//    uint8_t value = 0;
//    radio->read(&value, 1);
    siz = 0;
    return true;
}

bool RadioCommunication::receiveFromRadio()
{
    radio->startListening();
    if(radio->available())
    {
        siz = radio->getDynamicPayloadSize();
        if(siz < 1)
        {
            return false;
        }
        ack++;
        radio->read(received_data, siz);
        //radio->writeAckPayload(1, &ack, 1);
        if((received_data[0] == START_BYTE) && (received_data[siz - 1] == START_BYTE))
        {
            return true;
        }
    }
    return false;
}

void RadioCommunication::getData(unsigned char *data)
{
    int i = 0;
    for(i=0; i<siz; i++)
    {
        data[i] = received_data[i];
    }
}

int RadioCommunication::getDataSize()
{
    return siz;
}

void RadioCommunication::debug()
{
     radio->printDetails();
}

bool RadioCommunication::isChipConnected()
{
    return radio->isChipConnected();
} 
