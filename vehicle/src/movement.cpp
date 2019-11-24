#include "movement.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <pigpio.h>
#include "constants.hpp"

#define LEFT_BALANCE    (1.0f - balance)
#define RIGHT_BALANCE   (1.0f + balance) 
#define CMATERIAL 1.0f
#define D_SPEED 0.001

void tickP(void* args) {
    ((Movement*)args)->calculateSpeed();
}

Movement::Movement() :
    left_pid(consts::WheelType::left), right_pid(consts::WheelType::right),
    left_wheel(consts::WheelType::left), right_wheel(consts::WheelType::right)
{
    this->lr = 0.0f;
    this->rr = 0.0f;
    this->balance = 0.0f;
    this->lb = 1.0f;
    this->rb = 1.0f;
    this->l_dir = 0;
    this->r_dir = 0;
    this->turn_ticks = -1;
    this->initial_wheel_flag = false;
    this->lm_speed = 0.5;
    this->rm_speed = 0.5;

    gpioSetTimerFuncEx(8, 250, tickP, this);
    this->is_to_move = false;
    
}

void Movement::turn(const float degrees)
{
    this->left_wheel.stop();
    this->right_wheel.stop();
    int moved_right = 0;
    int moved_left = 0;
    int total_moved = 0;
	const int r_previous_read = this->right_wheel.encoder.ticks;
    const int l_previous_read = this->left_wheel.encoder.ticks;
    int holes_to_turn = std::rint((std::abs(degrees) / consts::vehicle_deg_per_hole) / 2.0f);
    std::cout << "Fazendo curva de " << holes_to_turn << " furos" << std::endl;
    int r_dir = (degrees > 0) ? 1 : -1;

    bool moving_left = true;
    bool moving_right = true;
    this->left_wheel.spin(-r_dir, consts::turn_speed);
    this->right_wheel.spin(r_dir, consts::turn_speed);
    while(total_moved < holes_to_turn)
    {
        if (moving_left && (moved_left >= holes_to_turn))
        {
            this->left_wheel.stop();
            moving_left = false;
            std::cout << "Curva: Parando roda da esquerda" << std::endl;
        }
        if (moving_right && (moved_right >= holes_to_turn))
        {
            this->right_wheel.stop();
            moving_right = false;
            std::cout << "Curva: Parando roda da direita" << std::endl;
        }
        moved_right = this->right_wheel.encoder.ticks - r_previous_read;
        moved_left = this->left_wheel.encoder.ticks - l_previous_read;
        total_moved = (moved_right + moved_left)/2;
        //std::cout << "ml: " << moved_left << " mr: " << moved_right << " total: " << total_moved <<std::endl;
        //std::cout << "l: " << this->left_wheel.encoder.ticks << " r: " << this->right_wheel.encoder.ticks << std::endl;
    }
    this->stop();
}

// Moves a single wheel for 
void Movement::turnOneWheel(const consts::WheelType wheel_type, const int direction, const float mm)
{
    this->stop();
    
    Wheel* wheel;
    if (wheel_type == consts::WheelType::left)
        wheel = &(this->left_wheel);
    else
        wheel = &(this->right_wheel);
    wheel->mmMovedSinceLastCall();

    wheel->spin(direction, consts::turn_speed);
    float total_moved = 0;
    while (total_moved < mm)
    {
        total_moved += wheel->mmMovedSinceLastCall();
   }
   wheel->stop();
}

// Tries to move in a straight line.
// Direction is 1 (forward) or -1 (backwards)
// Speed is in milimeters per second
// This function doesn't block execution
void Movement::goStraight(const int direction, const float speed)
{
    this->is_to_move = true;
    this->required_speed = speed;
    //Dir L = Dir R
    this->r_dir = direction * (speed > 0);
    this->l_dir = this->r_dir;
    this->left_wheel.spin(this->l_dir, this->lm_speed);
    this->right_wheel.spin(this->r_dir, this->rm_speed);
}


// Tries to move in a straight line for 'cm' centimeters
// Return the distance move by the left wheel minus the moved by the right wheel
// This function blocks execution
float Movement::goStraightMm(const int direction, const float mm, const float speed=300)
{
    std::cout << "Andando pra frente " << mm << " mm" << std::endl;
    float moved_left = 0;
    float moved_right = 0;
    this->left_wheel.mmMovedSinceLastCall();
    this->right_wheel.mmMovedSinceLastCall();
    this->goStraight(direction, speed);
    bool moving_left = true;
    bool moving_right = true;
    while((moved_left + moved_right) < 2*mm)
    {
        if (moving_left && moved_left > mm)
        {
            moving_left = false;
            this->left_wheel.stop();
        }      
        if (moving_right && moved_right > mm)
        {
            moving_right = false;
            this->right_wheel.stop();
        }
        moved_left += this->left_wheel.mmMovedSinceLastCall();
        moved_right += this->right_wheel.mmMovedSinceLastCall();
    }
    this->stop();
    moved_left += this->left_wheel.mmMovedSinceLastCall();
    moved_right += this->right_wheel.mmMovedSinceLastCall();
    return moved_left - moved_right;
}

void Movement::goCurve(const int direction, const float curvature) {
    //Dir L = a * Dir R
    //TODO: It
    float v0 = (lr + rr) / 2.0f;
    float w0 = v0 / curvature;

    this->lr = w0 * (curvature + consts::wheel_distance / 2.0f);
    this->rr = w0 * (curvature - consts::wheel_distance / 2.0f);
    
}

void Movement::stop()
{
    this->left_wheel.stop();
    this->right_wheel.stop();
    this->lr = 0;
    this->rr = 0;
    this->l_dir = 0;
    this->r_dir = 0;
    this->is_to_move = false;
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
                this->stop();
            }
            this->turn_ticks -= 1;
        }
    }
}

float Movement::getBalance(void) {
    return this->balance;
}

float Movement::setBalance(const float balance_) {  
    this->balance = std::clamp(balance_, -1.0f, 1.0f);

    if(this->balance > 0) {
        lb = 1.0f - this->balance;
        rb = 1.0f;
    }
    else {
        lb = 1.0f;
        rb = 1.0f + this->balance;
    }
    return this->balance;
}

bool Movement::isTurning(void) {
    return (this->turn_ticks > 0);
}

void Movement::calculateSpeed(void)
{
    if(this->is_to_move)
    {
        //std::cout << "l: " << this->left_wheel.getSpeed() << " send: "  << this->lm_speed << std::endl;
        //std::cout << "r: " << this->right_wheel.getSpeed() << " send: " << this->lm_speed << std::endl;
        const int l_error = this->left_wheel.getSpeed() - this->required_speed;
        const int r_error = this->right_wheel.getSpeed() - this->required_speed;
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

