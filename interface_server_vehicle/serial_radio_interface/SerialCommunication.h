//biblioteca responsavel por receber os dados do PC, e coloca-los num buffer local, para ser reenviados aos robos via rf

#ifndef _SERIALCOMMUNICATION_H_
#define _SERIALCOMMUNICATION_H_

#include <Arduino.h>
#include "TransConstants.h"

class SerialCommunication{
private:
    unsigned char received_data[W_DATA+W_ADDRESS];
    unsigned char sended_data[W_DATA+W_ADDRESS];
    int siz;

public:
    SerialCommunication();
    bool receiveFromSerial();
    void getAddress(uint8_t *address);
    void getData(unsigned char *data);
    int getDataSize();
    void sendToSerial(unsigned char *data, int _siz);
    ~SerialCommunication() {};
};
#endif
