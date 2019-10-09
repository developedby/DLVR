#include "Radio.h"

Radio::Radio() : RF24(RPI_V2_GPIO_P1_35, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ)
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

