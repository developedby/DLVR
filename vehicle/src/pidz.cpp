#include "pidz.hpp"
#include "wheel.hpp"
#include "constants.hpp"

PIDZ::PIDZ(const consts::WheelType wheel_type) {
    this->T = consts::pid_T_ms;
    if(wheel_type == consts::WheelType::left)
    {
        this->tune(consts::pid_l_Kp, consts::pid_l_Ki, consts::pid_l_Kd);
    }
    else
    {
        this->tune(consts::pid_r_Kp, consts::pid_r_Ki, consts::pid_r_Kd);
    }
    this->omin = 0.0f;
    this->omax =  1.0f;
}

void PIDZ::tune(const float Kp, const float Ki, const float Kd) {
    for(int i = 0; i < 3; ++i) {
        this->x[i] = 0.0f;
    }
    for(int i = 0; i < 2; ++i) {
        this->y[i] = 0.0f;
    }
    this->output = 0;
    //H[Z] = Y[Z] / X[Z]
    //Y[Z]
    this->nn[2] = T*(Ki*T + 2*Kp) + 4*Kd;   //Z^2
    this->nn[1] = 2*Ki*T*T - 8*Kd;          //Z^1
    this->nn[0] = T*(Ki*T - 2*Kp) + 4*Kd;   //Z^0
    //X[Z]
    this->dd[1] = 0;                        //Z^1
    this->dd[0] = -2*T;                     //Z^0
}

float PIDZ::push_error(const float e) {
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
    S = ((S < this->omax)?((S > this->omin)?S:this->omin):this->omax);
    for(int i = 1; i < 2; ++i) {
        this->y[i - 1] = this->y[i];
    }
    this->y[1] = S;
    this->output = S;
    return this->output;
}

float PIDZ::push_error(const float rf, const float fb) {
    return this->push_error(rf - fb);
}

float PIDZ::peek_output(void) {
    return this->output;
}
