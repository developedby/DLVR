#include "movement.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "constants.hpp"

#define LEFT_BALANCE    (1.0f - balance)
#define RIGHT_BALANCE   (1.0f + balance) 
#define CMATERIAL 1.0f

Movement::Movement() : left_pid(0), right_pid(1), left_wheel(0),  right_wheel(1)
{
    lr = 0.0f;
    rr = 0.0f;
    balance = 0.0f;
    lb = 1.0f;
    rb = 1.0f;
    l_dir = 0;
    r_dir = 0;
    turn_ticks = -1;
    initial_wheel_flag = false;
    wheel_distance = constants::vehicle_wheel_distance;
}

void Movement::turn(float degrees) {
    lr = 450;
    rr = 450;
    r_dir = 2*(degrees > 0) - 1;
    l_dir = -r_dir;                    //83.775804096
    this->turn_ticks = roundf(CMATERIAL * (abs(degrees * 83.775804096)/(4*lr/constants::vehicle_wheel_distance)) / constants::pid_T_ms);
}

// Tries to move in a straight line.
// Direction is 1 (forward) or -1 (backwards)
// Speed is in milimeters per second
// This function doesn't block execution
void Movement::goStraight(int direction, float speed){
    lr = abs(speed);
    rr = abs(speed);
    //Dir L = Dir R
    r_dir = l_dir = direction * (speed > 0);
}


// Tries to move in a straight line for 'cm' centimeters
// Return the distance move by the left wheel minus the moved by the right wheel
// This function blocks execution
float Movement::goStraightCm(int direction, float cm, float speed=300)
{
    float moved_left = 0;
    float moved_right = 0;
    this->left_wheel.cmMovedSinceLastCall();
    this->right_wheel.cmMovedSinceLastCall();
    this->goStraight(direction, speed);
    while((moved_left + moved_right) < cm)
    {
        moved_left += this->left_wheel.cmMovedSinceLastCall();
        moved_right += this->right_wheel.cmMovedSinceLastCall();
    }
    this->stop();
    return moved_left - moved_right;
}

void Movement::goCurve(int direction, float curvature) {
    //Dir L = a * Dir R
    //TODO: It
    float v0 = (lr + rr) / 2.0f;
    float w0 = v0 / curvature;

    this->lr = w0 * (curvature + wheel_distance / 2.0f);
    this->rr = w0 * (curvature - wheel_distance / 2.0f);
    
}

void Movement::stop()
{
    this->left_wheel.stop();
    this->right_wheel.stop();
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
    float l_dc = std::clamp(this->left_pid.push_error(lr * lb, aux), 0, 1);
    // Right
    aux = this->right_wheel.getSpeed();
    if (aux < 3.7f) {
        aux = 0.0;
    }
    else if(aux > 1000.0) {
        aux /= 2.0f;
    }
    float r_dc = std::clamp(this->right_pid.push_error(rr * rb, aux), 0, 1);
    // Adjust
    if(initial_wheel_flag) {
        this->right_wheel.spin(r_dir, r_dc);
        this->left_wheel.spin(l_dir, l_dc);
    }
    else {
        this->left_wheel.spin(l_dir, l_dc);
        this->right_wheel.spin(r_dir, r_dc);
    }
    this->initial_wheel_flag = !(this->initial_wheel_flag);
    if(this->turn_ticks >= 0) {
        if(this->turn_ticks == 0) {
            this->goStraight(0, 0);
        }
        this->turn_ticks -= 1;
    }
}

float Movement::getBalance(void) {
    return this->balance;
}

float Movement::setBalance(float balance_) {  
    this->balance = std::clamp(balance_, -1.0, 1.0);

    if(this->balance > 0) {
        lb = 1.0f - this->balance;
        rb = 1.0f;
    }
    else {
        lb = 1.0f;
        rb = 1.0f + this->balance;
    }
    return (this->balance);
}

bool Movement::isTurning(void) {
    return (this->turn_ticks > 0);
}

int 
