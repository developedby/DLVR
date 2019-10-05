#include "movement.hpp"

Movement::Movement(float wheel_distance, float l_Kp, float l_Ki, float l_Kd, float r_Kp, float r_Ki, float r_Kd,
                   int const l_enc_pin, int const l_motor_fwd_pin, int const l_motor_bkwd_pin, int const l_motor_pwm_pin,
                   int const r_enc_pin, int const r_motor_fwd_pin, int const r_motor_bkwd_pin, int const r_motor_pwm_pin) :
          wheel_distance(wheel_distance),
          left_pid(l_Kp, l_Ki, l_Kd),
          right_pid(r_Kp, r_Ki, r_Kd),
          left_wheel(l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin),
          right_wheel(r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin),
          lr(0.0f), rr(0.0f), l_dir(0), r_dir(0) {}

void Movement::turn(float degrees) {
    lr = speed;
    rr = speed;
    r_dir = 2*(degrees > 0) - 1;
    l_dir = -r_dir;
}

void Movement::goStraight(int direction, float speed){ //mmps
    lr = speed;
    ll = speed;
    //Dir L = Dir R
    r_dir = l_dir = direction;
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
    float lerr = this->lr - this->left_wheel.getSpeed();
    float l_dc = this->left_pid.push_error(lerr) * this->l_dc_mmps;
    // Right
    float rerr = this->right_wheel.getSpeed();
    float r_dc = this->left_pid.push_error(rerr) * this->r_dc_mmps;
    // Adjust
    this->left_wheel.spin(l_dir, l_dc);
    this->right_wheel.spin(r_dir, r_dc);
}