//biblioteca criada para escrever para um radio transmissor

#ifndef radio_communication_hpp_
#define radio_communication_hpp_

#include <pigpio.h>
#include "radio.hpp"
#include "message.hpp"
#include "constants.hpp"

class RadioCommunication
{
    unsigned char received_data[consts::radio_width_data];
    uint8_t last_address[consts::radio_width_address];
public:
    SentMessage last_sended_message;
    pthread_t *message_pooler;
    int size_;
    int attempts;
    Radio radio;
    bool has_new_msg;
    RadioCommunication();
    ~RadioCommunication() {}
    void newMessagePooling();
    void setAddress(uint8_t const *addres);
    void sendToRadio(SentMessage message);
    bool receiveFromRadio();
    ReceivedMessage getData();
    int getDataSize();
    void debug();
    bool isChipConnected();
};

void* newMessagePoolingThread(void* arg);

#endif
