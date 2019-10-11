//biblioteca feita para os sets que garantem o funcionamento do nRF24L01

#ifndef radiosets_hpp_
#define radiosets_hpp_

#include "TransConstants.h"
#include <RF24/RF24.h>

class Radio : public RF24{
public:
    Radio();
    ~Radio() {}
    void radioSets();
};

#endif
