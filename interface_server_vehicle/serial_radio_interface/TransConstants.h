#ifndef _TRANSCONSTANTS_H_
#define _TRANSCONSTANTS_H_

#define BDRATE 115200 //frequencia de comunicacao serial entre PC e Arduino
//pinagem nrf24l01 para arduino mega
//GND -- GND
//VCC -- 3.3V
//CE -- CE_PIN
//CSN -- CSN_PIN
//SCK -- digIO 52
//MOSI -- digIO 51
//MISO -- digIO 50
//IRQ -- NONE

//pinagem FTDI232
//GND -- GND
//CTS -- NONE
//VCC -- 5V
//TX -- RX1
//RX -- TX1
//DTR -- NONE
//JUMPER - 5V

//sets para funcionamento do nRF24L01. Documentacao em http://tmrh20.github.io/RF24/classRF24.html#a5e6e5a5f6c85d2638381cab2c0f3702e
#define CE_PIN 7
#define CSN_PIN 8
#define DELAY_TRANS 0 //delay de transmissao
#define RETRIES 3 //numero de tentativas de envio para os receptores
#define CHANNEL 33 //canal RF de comunicacao, 0-125 (2400+CHANNEL MHZ)
#define W_ADDRESS 5 //quantidade de bytes do endereco
#define W_DATA 45 //quantidade de bytes do pacote para envio - 3 velocidades 1 nome e 1 inicio
#define DATA_RATE RF24_2MBPS //velocidade de transmissao de dados

//chaves de comunicacao
#define START_BYTE 0xAA //chave que indica o inicio de um novo pacote contendo endereco e dados para os robos receptores
#define COORD_ADDRESS "DLVRC" //chave que indica o inicio de um pacote efetivo de velocidades
#define TIME_OUT_RADIO_ACK 1000


#endif
