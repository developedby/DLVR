//biblioteca responsavel por receber os dados do transmissor, e coloca-los num buffer local, para ser transformado posteriormente em velocidades angular, x e y

#ifndef _RFRECEIVER_HPP_
#define _RFRECEIVER_HPP_

#include <Arduino.h>
#include "BotConstants.h"
#include "Buffer.hpp"
#include "Radio.hpp"

class RFreceiver{
private:
    float vel_x;
    float vel_y;
    float vel_ang;
    Buffer* queue;
    Radio *radio;
    uint8_t address;

public:
    RFreceiver();
    int updateBuffer();
    float getVelX();
    float getVelY();
    float getVelAng();
    void receiveData();
    void debug();
    ~RFreceiver() {};
};
#endif
