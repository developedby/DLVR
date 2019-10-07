#ifndef _PRINCIPAL_H_
#define _PRINCIPAL_H_

#include <Arduino.h>
#include "SerialCommunication.h"
#include "RadioCommunication.h"

class Principal{
    SerialCommunication *serial;
    RadioCommunication *radio;
public:
    Principal();
    ~Principal() {}
    void start();
};

#endif
