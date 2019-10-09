//biblioteca criada para escrever para um radio transmissor

#ifndef _RADIOCOMMUNICATION_H_
#define _RADIOCOMMUNICATION_H_

#include "Radio.h"

class RadioCommunication{
    Radio *radio;
    unsigned char received_data[W_DATA];
    int siz;
    uint8_t last_address[W_ADDRESS];
    int ack;
    pthread_t *radio_ack_thread;
    unsigned char *last_sended_data;
    int last_sended_data_size;
    
public:
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

void waitAck(void *);
void sendOk(int event, uint32_t tick, void *userdata);
void sendFailure(int event, uint32_t tick, void *userdata);

#endif
