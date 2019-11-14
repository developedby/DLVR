#include "movement.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <pigpio.h>
#include "constants.hpp"

#define LEFT_BALANCE    (1.0f - balance)
#define RIGHT_BALANCE   (1.0f + balance) 
#define CMATERIAL 1.0f
#define D_SPEED 0.0015

void tickPID(void* args) {
	((Movement*)args)->tick();
}

void tickP(void* args) {
    ((Movement*)args)->calculateSpeed();
}

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
    lm_speed = 0.5;
    rm_speed = 0.5;
    wheel_distance = constants::vehicle_wheel_distance;
    gpioSetTimerFuncEx(9, constants::pid_T_ms, tickPID, this);
    gpioSetTimerFuncEx(8, 250, tickP, this);
    
}

void Movement::turn(float degrees) {
    lr = 450;
    rr = 450;
    r_dir = 2*(degrees > 0) - 1;
    l_dir = -r_dir;                    //83.775804096
    this->turn_ticks = roundf(CMATERIAL * (abs(degrees * 83.775804096)/(4*lr/constants::vehicle_wheel_distance)) / constants::pid_T_ms);
    while(this->isTurning());
}

// Tries to move in a straight line.
// Direction is 1 (forward) or -1 (backwards)
// Speed is in milimeters per second
// This function doesn't block execution
void Movement::goStraight(int direction, float speed){
    this->required_speed = speed;
    //Dir L = Dir R
    r_dir = l_dir = direction * (speed > 0);
}


// Tries to move in a straight line for 'cm' centimeters
// Return the distance move by the left wheel minus the moved by the right wheel
// This function blocks execution
float Movement::goStraightMm(int direction, float mm, float speed=300)
{
    float moved_left = 0;
    float moved_right = 0;
    this->left_wheel.mmMovedSinceLastCall();
    this->right_wheel.mmMovedSinceLastCall();
    this->goStraight(direction, speed);
    while((moved_left + moved_right) < 2*mm)
    {
        if (moved_left > mm)
            lr = 0;
        else
            moved_left += this->left_wheel.mmMovedSinceLastCall();
        if (moved_right > mm)
            rr = 0;
        else
            moved_right += this->right_wheel.mmMovedSinceLastCall();
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
    this->lr = 0;
    this->rr = 0;
    this->l_dir = 0;
    this->r_dir = 0;
}

void Movement::tick(void) {
    if(this->turn_ticks >= 0)
    {
        float aux;
        // Left
        aux = this->left_wheel.getSpeed();
        if (aux < 3.7f) {
            aux = 0.0;
        }
        else if(aux > 1000.0) {
            aux /= 2.0f;
        }
        float l_dc = std::clamp(this->left_pid.push_error(lr * lb, aux), 0.0f, 1.0f);
        // Right
        aux = this->right_wheel.getSpeed();
        if (aux < 3.7f) {
            aux = 0.0;
        }
        else if(aux > 1000.0) {
            aux /= 2.0f;
        }
        float r_dc = std::clamp(this->right_pid.push_error(rr * rb, aux), 0.0f, 1.0f);
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
}

float Movement::getBalance(void) {
    return this->balance;
}

float Movement::setBalance(float balance_) {  
    this->balance = std::clamp(balance_, -1.0f, 1.0f);

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

void Movement::calculateSpeed(void)
{
    if(this->turn_ticks < 0)
    {
        int l_error = this->left_wheel.getSpeed() - this->required_speed;
        int r_error = this->right_wheel.getSpeed() - this->required_speed;
        this->lm_speed -= l_error*D_SPEED;
        this->rm_speed -= r_error*D_SPEED;
        if (l_error > r_error) //roda esquerda esta mais rapida
        {
            this->lm_speed -= 0.01;
        }
        else if(r_error > l_error)
        {
            this->rm_speed -= 0.01;
        }
        if(lm_speed < 0)
        {
            this->lm_speed = 0;
        }
        else if(lm_speed > 1)
        {
            this->lm_speed = 1;
        }
        if(rm_speed < 0)
        {
            this->rm_speed = 0;
        }
        else if(rm_speed > 1)
        {
            this->rm_speed = 1;
        }
        this->left_wheel.spin(l_dir, lm_speed);
        this->right_wheel.spin(r_dir, rm_speed);
    }
}

