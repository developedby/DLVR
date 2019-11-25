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
    this->moving_left = false;
    this->moving_right = false;
    
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

    bool internal_moving_left = true; //this function don't use calculate speed
    bool internal_moving_right = true;
    this->left_wheel.spin(-r_dir, consts::turn_speed);
    this->right_wheel.spin(r_dir, consts::turn_speed);
    while(total_moved < holes_to_turn)
    {
        if (internal_moving_left && (moved_left >= holes_to_turn))
        {
            this->left_wheel.stop();
            internal_moving_left = false;
            std::cout << "Curva: Parando roda da esquerda" << std::endl;
        }
        if (internal_moving_right && (moved_right >= holes_to_turn))
        {
            this->right_wheel.stop();
            internal_moving_right = false;
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
   this->stop();
}

// Tries to move in a straight line.
// Direction is 1 (forward) or -1 (backwards)
// Speed is in milimeters per second
// This function doesn't block execution
void Movement::goStraight(const int direction, const float speed)
{
    this->moving_left = true;
    this->moving_right = true;
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
float Movement::goStraightMm(const int direction, float mm, const float speed=300)
{
    std::cout << "Andando pra frente " << mm << " mm" << std::endl;
    mm = std::clamp(mm - 20, 0.0f, mm);
    float moved_left = 0;
    float moved_right = 0;
    this->left_wheel.mmMovedSinceLastCall();
    this->right_wheel.mmMovedSinceLastCall();
    this->goStraight(direction, speed);
    std::cout << "antes l furo " << this->left_wheel.encoder.ticks << " l diferenca: " << moved_left << std::endl;
    std::cout << "antes R furo " << this->right_wheel.encoder.ticks << " r diferenca: " << moved_right << std::endl;
    while((moved_left + moved_right) < 2*mm)
    {
        if (moving_left && moved_left > mm)
        {
            this->moving_left = false;
            this->left_wheel.stop();
            //std::cout << "parou esquerda" << std::endl;
        }      
        if (moving_right && moved_right > mm)
        {
            this->moving_right = false;
            this->right_wheel.stop();
            //std::cout << "parou direita" << std::endl;
        }
        moved_left += this->left_wheel.mmMovedSinceLastCall();
        moved_right += this->right_wheel.mmMovedSinceLastCall();        
    }
    std::cout << "l furo" << this->left_wheel.encoder.ticks << " l diferenca: " << moved_left << std::endl;
    std::cout << "R furo" << this->right_wheel.encoder.ticks << " r diferenca: " << moved_right << std::endl;
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
    /*this->l_dir = -this->l_dir;
    this->r_dir = -this->r_dir;
    this->left_wheel.spin(this->l_dir, this->lm_speed);
    this->right_wheel.spin(this->r_dir, this->rm_speed);
    gpioDelay(50000);*/
    this->moving_left = false;
    this->moving_right = false;
    this->left_wheel.stop();
    this->right_wheel.stop();
    this->lr = 0;
    this->rr = 0;
    this->l_dir = 0;
    this->r_dir = 0;
}

void Movement::calculateSpeed(void)
{
    if(this->moving_left || this->moving_right)
    {
        //std::cout << "l: " << this->left_wheel.getSpeed() << " send: "  << this->lm_speed << std::endl;
        //std::cout << "r: " << this->right_wheel.getSpeed() << " send: " << this->lm_speed << std::endl;
        const int l_error = this->left_wheel.getSpeed() - this->required_speed;
        const int r_error = this->right_wheel.getSpeed() - this->required_speed;
        this->lm_speed -= (this->moving_left) ? l_error*D_SPEED : 0;
        this->rm_speed -= (this->moving_right) ? r_error*D_SPEED : 0;
        if(this->moving_left && this->moving_right)
        {
            if (l_error > r_error) //roda esquerda esta mais rapida
            {
                this->lm_speed -= 0.01;
            }
            else if(r_error > l_error)
            {
                this->rm_speed -= 0.01;
            }
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
        this->left_wheel.spin(this->l_dir, this->lm_speed);
        this->right_wheel.spin(this->r_dir, this->rm_speed);
    }
}

