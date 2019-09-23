#ifndef _TRANSCONSTANTS_H_
#define _TRANSCONSTANTS_H_

#define BDRATE 115200 //frequency of communication arduino and pc
//pinout nrf24l01 to arduino nano
//GND -- GND
//VCC -- 3.3V
//CE -- CE_PIN
//CSN -- CSN_PIN
//SCK -- digIO 13
//MOSI -- digIO 11
//MISO -- digIO 12
//IRQ -- NONE

//sets to do nRF24L01 working. Documentation on http://tmrh20.github.io/RF24/classRF24.html#a5e6e5a5f6c85d2638381cab2c0f3702e
#define CE_PIN 7
#define CSN_PIN 8
#define DELAY_TRANS 0 //delay de transmissao
#define RETRIES 3 //numero de tentativas de envio para os receptores
#define CHANNEL 33 //canal RF de comunicacao, 0-125 (2400+CHANNEL MHZ)
#define W_ADDRESS 5 //quantidade de bytes do endereco
#define W_DATA 16 //quantidade de bytes do pacote para envio
#define DATA_RATE RF24_2MBPS //velocidade de transmissao de dados


#endif
