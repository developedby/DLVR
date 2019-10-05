#include "pidz.hpp"

PIDZ::PIDZ(float Kp, float Ki, float Kd, float T) {
    this->a[2] = Kp + 2*Kd/T + Ki*T/2
    this->a[1] = Ki*T - 4*Kd/T
    this->a[0] = Ki*T/2 + 2*Kd/T - Kp
    for(int i = 0; i < 3; ++i) {
        this->x[i] = 0.0f
    }
    this->y = 0.0f;
    this->insert = 0;
}

float PIDZ::push_error(float e) {
    this->x[(this->insert+1)%3] = e
    this->y = 0;
    for(int i = 0; i < 3; ++i) {
        this->y += this->a[2 - i] * this->x[(this->insert - i + 3)%3]
    }
    this->insert++;
    return this->y;
}

float PIDZ::peek_output(void) {
    return this->y;
}