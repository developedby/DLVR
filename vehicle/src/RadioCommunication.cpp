#include "RadioCommunication.h"
#include <iostream>

RadioCommunication * internal_radio;
RadioCommunication::RadioCommunication()
{
    internal_radio = this;
    int i = 0;
    radio = new Radio();
    for(i=0; i<W_DATA; i++)
    {
        received_data[i] = 0;
    }
    siz = 0;
    ack = 0;
    uint8_t address[] = {MY_ADDRESS};
    radio->openReadingPipe(1, address);
    radio->enableAckPayload();               // Allow optional ack payloads
    radio->enableDynamicPayloads();
    radio->startListening();
    eventSetFunc(1, sendFailure);
    eventSetFunc(2, sendOk);
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

void RadioCommunication::sendToRadio(const void *data, uint8_t len)
{
    radio->stopListening();
    radio->write(data, len);
    radio->startListening();
    radio_ack_thread = gpioStartThread(waitAck, nullptr);
}

void waitAck(void *)
{
    uint32_t startTick = gpioTick();
    while(!radio->isAckPayloadAvailable())
    {
        if((gpioTick() - startTick) > TIME_OUT_RADIO_ACK)
        {
            eventTrigger(1);
            return
        }
    }
    eventTrigger(2);
}

void sendOk(int event, uint32_t tick, void *userdata)
{
    internal_radio->radio->writeAckPayload(1, &(internal_radio->ack), 1);
    internal_radio->siz = 0;
    internal_radio->attemps = 0;
    gpioStopThread(radio_ack_thread);
}

void sendFailure(int event, uint32_t tick, void *userdata)
{
    internal_radio->attemps++;
    gpioStopThread(radio_ack_thread);
    if(internal_radio->attemps > RETRIES)
    {
        eventTrigger(3);
    }
    else
    {
        internal_radio->sendToRadio(internal_radio->last_sended_data, internal_radio->last_sended_data_size);
    }
}

bool RadioCommunication::receiveFromRadio()
{
    if(radio->available())
    {
        //std::cout <<"ok";
        if(radio->getDynamicPayloadSize() < 1)
        {
            return false;
        }
        //std::cout <<"ok"<<std::endl;
        radio->read(received_data, sizeof(received_data));
        siz = 2;
        for (int i=1; received_data[i] != START_BYTE and i < W_DATA; i++)
        {
            siz++;
        }
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
