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
    
public:
    RadioCommunication();
    ~RadioCommunication() {}
    void setAddress(uint8_t *addres);
    bool sendToRadio(const void *data, uint8_t len);
    bool receiveFromRadio();
    void getData(unsigned char *data);
    int getDataSize();
    void debug();
    bool isChipConnected();
};

#endif
