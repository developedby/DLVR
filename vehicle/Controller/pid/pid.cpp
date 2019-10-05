#include "pid.hpp"

PID::PID(float Kp, float Ki, float Kd, float T) : 
     Kp(Kp), Ki(Ki), Kd(Kd), T(T), integrate(0.0f), last_error(0.0f) {}

PIP::~PID() {}

float PID::push_error(float e) {
    float P, I, D;
    
    
    // Proportional
    P = this->Kp * e;
    
    // Integral
    this->integrate += e * this->T;
    I = this->Ki * this->integrate;
    
    // Derivative
    D = this->Kd * (e - this->last_error)/this->T;
    
    // Store last error
    this->last_error = e;
    
    // Output
    this->y = (P + I + D)
    return this->y;
    
}

float PID::peek_output(void) {
    return this->y;
}

void PID::reset() {
    this->last_error = 0.0f;
    this->integrate = 0.0f;
}

void PID::set_pid(float Kp, float Ki, float Kd, float T) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
    this->T = T;
    this->reset();
}

float PID::get_Kp() {
    return this->Kp;
}

float PID::get_Ki() {
    return this->Ki;
}

float PID::get_Kd() {
    return this->Kd;
}

float PID::get_T() {
    return this->T;
}

std::ostream& operator<<(std::ostream &strm, const PID &pid) {
    return strm << "PID(Kp=" << pid.Kp << ", Ki=" << pid.Ki << ", Kd=" << pid.Kd << ")";
}
