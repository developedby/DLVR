//biblioteca criada para escrever para um radio transmissor

#ifndef _RADIOWRITER_H_
#define _RADIOWRITER_H_

#include "Radio.h"

class RadioWriter{
    Radio *radio;
public:
    RadioWriter();
    ~RadioWriter() {}
    void setAddress(const uint8_t* addres);
    void setChannel(char name);
    void sendData(const void* data);
    void debug();
};

#endif
