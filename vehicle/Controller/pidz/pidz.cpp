#include "pidz.hpp"
#include <iostream>

PIDZ::PIDZ(float Kp, float Ki, float Kd, float T) {
    this->nn[2] = T*(Ki*T + 2*Kp) + 4*Kd;
    this->nn[1] = 2*Ki*T*T - 8*Kd;
    this->nn[0] = T*(Ki*T - 2*Kp) + 4*Kd;
    
    this->dd[1] =   0;
    this->dd[0] =  -2*T;
    
    for(int i = 0; i < 3; ++i) {
        this->x[i] = 0.0f;
    }
    for(int i = 0; i < 2; ++i) {
        this->y[i] = 0.0f;
    }
    
    this->T = T;
    this->output = 0;
}

float PIDZ::push_error(float e) {
    float S = 0;
    for(int i = 1; i < 3; ++i) {
        this->x[i - 1] = this->x[i];
    }
    this->x[2] = e;
    //
    for(int i = 0; i < 3; ++i) {
        S += this->nn[i] * this->x[i];
    }
    for(int i = 0; i < 2; ++i) {
        S -= this->dd[i] * this->y[i];
    }
    S = S/(2 * this->T);
    //
    for(int i = 1; i < 2; ++i) {
        this->y[i - 1] = this->y[i];
    }
    this->y[1] = S;
    this->output += S;
    return this->output;
}

float PIDZ::peek_output(void) {
    return this->output;
}