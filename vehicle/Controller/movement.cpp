#include "movement.hpp"
#include <iostream>

#define max(a,b) ((a > b)?a:b)
#define min(a,b) ((a < b)?a:b)
#define abs(a) ((a >= 0)?a:-a)

Movement::Movement(float wheel_distance, float l_Kp, float l_Ki, float l_Kd, float r_Kp, float r_Ki, float r_Kd, float T,
                   int const l_enc_pin, int const l_motor_fwd_pin, int const l_motor_bkwd_pin, int const l_motor_pwm_pin,
                   int const r_enc_pin, int const r_motor_fwd_pin, int const r_motor_bkwd_pin, int const r_motor_pwm_pin) :
          wheel_distance(wheel_distance),
          left_pid(l_Kp, l_Ki, l_Kd, T),
          right_pid(r_Kp, r_Ki, r_Kd, T),
          left_wheel(l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin),
          right_wheel(r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin),
          lr(0.0f), rr(0.0f), l_dir(0), r_dir(0) {}


void Movement::turn(float degrees) {
    lr = 10;
    rr = 10;
    r_dir = 2*(degrees > 0) - 1;
    l_dir = -r_dir;
}

void Movement::goStraight(int direction, float speed){ //mmps
    lr = abs(speed);
    rr = abs(speed);
    //Dir L = Dir R
    r_dir = l_dir = direction * (speed > 0);
}

void Movement::goCurve(int direction, float curvature) {
    //Dir L = a * Dir R
    //TODO: It
    float v0 = (lr + rr) / 2.0f;
    float w0 = v0 / curvature;

    this->lr = w0 * (curvature + wheel_distance / 2.0f);
    this->rr = w0 * (curvature - wheel_distance / 2.0f);
    
}

void Movement::tick() {
    // Left
    float lmin = (this->lr > 0.0f)?0.2f:0.0f;
    float aux = this->left_wheel.getSpeed();
    std::cout << "Lreadvel: " << aux << std::endl;
    aux = (aux <= 3.7f)?0.0f:aux;
    float lerr = this->lr -  aux;
    float l_dc = this->left_pid.push_error(lerr);
    std::cout << l_dc << " " << min(l_dc, 1.0) << " " << max(0.0f, min(l_dc, 1.0f)) << std::endl;
    l_dc = max(lmin, min(l_dc, 1.0f));
    // Right
    float rmin = (this->rr > 0.0f)?0.2f:0.0f;
    aux = this->right_wheel.getSpeed();
    std::cout << "Rreadvel: " << aux << std::endl;
    aux = (aux <= 3.7f)?0.0f:aux;
    float rerr = this->rr - aux;
    float r_dc = this->left_pid.push_error(rerr);
    r_dc = max(rmin, min(r_dc, 1.0f));
    // Adjust
    this->left_wheel.spin(l_dir, l_dc);
    this->right_wheel.spin(r_dir, r_dc);
    std::cout << "Left" << std::endl;
    std::cout << "    " <<"e:" << lerr << " r:" << this->lr << " dc: " << l_dc << std::endl;
    std::cout << "Right" << std::endl;
    std::cout << "    " <<"e:" << rerr << " r:" << this->rr << " dc: " << r_dc << std::endl << std::endl;
}
