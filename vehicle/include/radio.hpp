//biblioteca feita para os sets que garantem o funcionamento do nRF24L01

#ifndef radio_hpp_
#define radio_hpp_

#include <RF24/RF24.h>

#define DATA_RATE RF24_2MBPS //velocidade de transmissao de dados
class Radio : public RF24{
public:
    Radio();
    ~Radio() {}
    void radioSets();
};

#endif
