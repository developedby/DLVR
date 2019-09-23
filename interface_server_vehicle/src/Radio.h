//biblioteca feita para os sets que garantem o funcionamento do nRF24L01

#ifndef _RADIOSETS_H_
#define _RADIOSETS_H_

#include <SPI.h>
#include "TransConstants.h"
#include <nRF24L01.h>
#include <RF24.h>

class Radio : public RF24{
public:
    Radio();
    ~Radio() {}
    void radioSets();
};

#endif
