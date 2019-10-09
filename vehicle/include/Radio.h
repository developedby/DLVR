//biblioteca feita para os sets que garantem o funcionamento do nRF24L01

#ifndef _RADIOSETS_H_
#define _RADIOSETS_H_

#include "TransConstants.h"
#include <RF24/RF24.h>

class Radio : public RF24{
public:
    Radio();
    ~Radio() {}
    void radioSets();
};

#endif
