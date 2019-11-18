#include "radio.hpp"
#include "constants.hpp"

Radio::Radio() : RF24(RPI_V2_GPIO_P1_35, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ)
{
    radioSets();
}
void Radio::radioSets()
{
    begin();
    setRetries(consts::radio_delay_transmitting, consts::radio_retries);
    //setChannel(CHANNEL);
    setAddressWidth(consts::radio_width_address);
    //setPayloadSize(W_DATA);
    setDataRate(DATA_RATE);
}

