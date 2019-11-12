//biblioteca criada para escrever para um radio transmissor

#ifndef radio_communication_hpp_
#define radio_communication_hpp_

#include <pigpio.h>
#include "radio.hpp"
#include "message.hpp"
#include "constants.hpp"

class RadioCommunication{
    unsigned char received_data[constants::radio_width_data];
    uint8_t last_address[constants::radio_width_address];

public:
    SentMessage last_sended_message;
    pthread_t *radio_ack_thread;
    int siz;
    int ack;
    int attemps;
    Radio radio;
    RadioCommunication();
    ~RadioCommunication() {}
    void setAddress(uint8_t const *addres);
    void sendToRadio(SentMessage message);
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
