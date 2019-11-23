#include "wheel.hpp"
#include <cmath>
#include <iostream>
#include <pigpio.h>
#include "constants.hpp"


Wheel::Wheel(int wheel_num) : dc_motor(wheel_num), last_encoder_ticks(0), encoder(wheel_num) {}

void Wheel::spin(int const direction, float const duty_cycle)
{
    //std::cout << "Girando direcao " << direction << " dc " << duty_cycle << std::endl;
    this->dc_motor.spin(direction, duty_cycle);
}

float Wheel::getSpeed()
{
    return this->encoder.getAngularSpeed() * consts::wheel_radius_mm;
}

void Wheel::stop()
{
    this->dc_motor.spin(0, 0);
    this->encoder.resetReadings();
}

// Returns the number of centimenters moved since this function was last called
// This function doesn't differentiate moving forwards and backwards
float Wheel::mmMovedSinceLastCall()
{
    const int num_ticks = this->encoder.ticks - this->last_encoder_ticks;
    this->last_encoder_ticks = this->encoder.ticks;
    return num_ticks * ((2*M_PI)/consts::num_holes_encoder) * consts::wheel_radius_mm; 
}
