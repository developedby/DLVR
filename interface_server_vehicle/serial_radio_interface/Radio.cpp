#include "Radio.h"

Radio::Radio() : RF24(CE_PIN, CSN_PIN)
{
    radioSets();
}
void Radio::radioSets()
{
    begin();
    setRetries(DELAY_TRANS, RETRIES);
    //setChannel(CHANNEL);
    setAddressWidth(W_ADDRESS);
    //setPayloadSize(W_DATA);
    setDataRate(DATA_RATE);
}
