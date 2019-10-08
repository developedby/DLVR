#include "pid.hpp"

#ifndef min
    #define min(a, b) ((a < b)?a:b)
#endif
#ifndef max
    #define max(a, b) ((a > b)?a:b)
#endif
 
PID::PID(float Kp, float Ki, float Kd, float T) :
     Kp(Kp), Ki(Ki), Kd(Kd), T(T), integrate(0.0f), last_error(0.0f),
     y(0.0f), minv(-HUGE_VALF), maxv(HUGE_VALF) {}

PID::PID(float Kp, float Ki, float Kd, float T, float min, float max) :
     Kp(Kp), Ki(Ki), Kd(Kd), T(T), integrate(0.0f), last_error(0.0f),
     y(0.0f), minv(min), maxv(max) {}

void PID::tunning(float Kp, float Ki, float Kd) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
    this->integrate = 0.0f;
    this->last_error = 0.0f;
}

float PID::push_error(float e) {
    float P, I, D, aux;
    // Proportional
    P = this->Kp * e;
    
    // Integrate
    this->integrate += e * this->T * this->Ki;
    I = this->integrate;
    
    // Derivative
    D = this->Kd * ((e - this->last_error) / this->T);
    this->last_error = e;
    
    aux = P + I + D;
    /**/
    aux += this->y;
    /**/
    this->y = min(this->maxv, max(this->minv, aux));

    return (this->y);
}

float PID::push_error(float r, float y) {
    return this->push_error(r - y);
}

float PID::push_setpoint(float sp) {
    return this->push_error(sp - this->y);
}

float PID::peek_output() {
    return (this->y);
}

