#include "movement.hpp"
#include <iostream>

#define abs(a) ((a >= 0)?a:-a)
#define LEFT_BALANCE    ((1 - balance)/2)
#define RIGHT_BALANCE   ((1 + balance)/2) 

float Movement::limit(float vmin, float v, float vmax) {
    return min(vmax, max(vmin, v));
}

Movement::Movement(float wheel_distance, float l_Kp, float l_Ki, float l_Kd, float r_Kp, float r_Ki, float r_Kd, float T) :
          wheel_distance(wheel_distance), iwflag(false)
          left_pid(l_Kp, l_Ki, l_Kd, T, 0.0f, 1.0f),
          right_pid(r_Kp, r_Ki, r_Kd, T, 0.0f, 1.0f),
          lr(0.0f), rr(0.0f), balance(0.0f), l_dir(0), r_dir(0) 
{
    left_wheel(0);
    right_wheel(1);
}

void Movement::turn(float degrees) {
    lr = 800;
    rr = 800;
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

    this->lr = w0 * (curvature + wheel_distance / 2.0f) * LEFT_BALANCE;
    this->rr = w0 * (curvature - wheel_distance / 2.0f) * RIGHT_BALANCE;
    
}

void Movement::tick(void) {
    float aux;
    // Left
    aux = this->left_wheel.getSpeed();
    if (aux < 3.7f) {
        aux = 0.0;
    }
    else if(aux > 1000.0) {
        aux /= 2.0f;
    }
    float l_dc = this->limit(this->left_pid.push_error(lr * LEFT_BALANCE, aux));
    // Right
    aux = this->right_wheel.getSpeed();
    if (aux < 3.7f) {
        aux = 0.0;
    }
    else if(aux > 1000.0) {
        aux /= 2.0f;
    }
    float r_dc = this->right_pid.push_error(rr * RIGHT_BALANCE, aux);
    // Adjust
    if(iwflag) {
        this->right_wheel.spin(r_dir, r_dc);
        this->left_wheel.spin(l_dir, l_dc);
    }
    else {
        this->left_wheel.spin(l_dir, l_dc);
        this->right_wheel.spin(r_dir, r_dc);
    }
    this->iwflag = !(this->iwflag);
}

float getBalance(void) {
    return (this->balance);
}

float setBalance(float balance) {
    this->balance = ((balance < 1.0)?((balance > -1.0)?balance:(-1.0)):(1.0))
    return (this->balance);
}
