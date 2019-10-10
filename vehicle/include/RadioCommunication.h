//biblioteca criada para escrever para um radio transmissor

#ifndef _RADIOCOMMUNICATION_H_
#define _RADIOCOMMUNICATION_H_

#include "Radio.h"
#include "message.h"
#include <pigpio.h>

class RadioCommunication{
    unsigned char received_data[W_DATA];
    uint8_t last_address[W_ADDRESS];
    //pthread_t *radio_ack_thread;
    
public:
    SendedMessage last_sended_message;
    pthread_t *radio_ack_thread; 
    int siz;
    int ack;
    int attemps;
    Radio radio;
    RadioCommunication();
    ~RadioCommunication() {}
    void setAddress(uint8_t *addres);
    void sendToRadio(SendedMessage message);
    bool receiveFromRadio();
    ReceivedMessage getData();
    int getDataSize();
    void debug();
    bool isChipConnected();
};

void* waitAck(void *);
void sendOk(int event, uint32_t tick, void *obj);
void sendFailure(int event, uint32_t tick, void *obj);

#endif
