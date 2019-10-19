#include "movement.hpp"
#include <iostream>
#include "constants.hpp"

#define abs(a) ((a >= 0)?a:-a)
#define LEFT_BALANCE    ((1 - balance)/2)
#define RIGHT_BALANCE   ((1 + balance)/2) 

float Movement::limit(float vmin, float v, float vmax) {
    return ((v > vmax)?(vmax):((v < vmin)?(vmin):(v)));
}

Movement::Movement() :
          left_pid(0), right_pid(1),
          left_wheel(0),  right_wheel(1)
{
    lr = 0.0f;
    rr = 0.0f;
    balance = 0.0f;
    l_dir = 0;
    r_dir = 0;
    turn_ticks = -1;
    iwflag = false;
    wheel_distance = constants::vehicle_wheel_distance;
}

void Movement::turn(float degrees) {
    lr = 450;
    rr = 450;
    r_dir = 2*(degrees > 0) - 1;
    l_dir = -r_dir;
    this->turn_ticks = roundf((abs(degrees * 83.775804096)/(2*lr/constants::vehicle_wheel_distance)) / constants::pid_T_ms);
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
    float l_dc = this->limit(0, this->left_pid.push_error(lr * LEFT_BALANCE, aux), 1);
    // Right
    aux = this->right_wheel.getSpeed();
    if (aux < 3.7f) {
        aux = 0.0;
    }
    else if(aux > 1000.0) {
        aux /= 2.0f;
    }
    float r_dc = this->limit(0, this->right_pid.push_error(rr * RIGHT_BALANCE, aux), 1);
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
    if(this->turn_ticks >= 0) {
        if(this->turn_ticks == 0) {
            this->goStraight(0, 0);
        }
        --this->turn_ticks;
    }
}

float Movement::getBalance(void) {
    return (this->balance);
}

float Movement::setBalance(float balance) {
    this->balance = ((balance < 1.0)?((balance > -1.0)?balance:(-1.0)):(1.0));
    return (this->balance);
}

bool Movement::isTurning(void) {
    return (this->turn_ticks > 0);
}

