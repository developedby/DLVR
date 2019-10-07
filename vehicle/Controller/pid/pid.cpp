#include "pid.hpp"

PID::PID(float Kp, float Ki, float Kd, float T) : 
     Kp(Kp), Ki(Ki), Kd(Kd), T(T), integrate(0.0f), last_error(0.0f) {}

PID::~PID() {}

float PID::push_error(float e) {
    float P, I, D, aux;
    
    
    // Proportional
    P = this->Kp * e;
    
    // Integral
    aux = this->integrate + e * this->T;
    I = this->Ki * aux;
    
    // Derivative
    D = this->Kd * (e - this->last_error)/this->T;
    
    // Store last error
    this->last_error = e;
    
    // Output
    this->y = (P + I + D) * 0.003f;
    if(this->y < 0.2) {
        this->y = 0.2;
    }
    else if(this-> y > 1.0) {
        this->y = 1.0;
    }
    else {
        this->integrate = aux;
    }
    return this->y;
    
}

float PID::peek_output(void) {
    return this->y;
}

void PID::reset() {
    this->last_error = 0.0f;
    this->integrate = 0.0f;
}


