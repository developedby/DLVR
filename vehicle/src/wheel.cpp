#include "wheel.hpp"
#include <cmath>
#include <iostream>
#include <pigpio.h>
#include "constants.hpp"


Wheel::Wheel(const consts::WheelType wheel_type) :
    dc_motor(wheel_type), last_encoder_ticks(0), encoder(wheel_type)
{}

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
    //this->dc_motor.lock();
    this->dc_motor.release();
    this->encoder.resetReadings();
}

// Returns the number of centimenters moved since this function was last called
// This function doesn't differentiate moving forwards and backwards
float Wheel::mmMovedSinceLastCall()
{
    const int num_ticks = this->encoder.ticks - this->last_encoder_ticks;
    this->last_encoder_ticks = this->encoder.ticks;
    return num_ticks * consts::mm_moved_per_hole;
}
