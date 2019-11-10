#include "wheel.hpp"
#include <cmath>
#include <iostream>
#include <pigpio.h>


Wheel::Wheel(int wheel_num) : encoder(wheel_num), dc_motor(wheel_num) {}

void Wheel::spin(int const direction, float const duty_cycle)
{
    //std::cout << "Girando direcao " << direction << " dc " << duty_cycle << std::endl;
    this->dc_motor.spin(direction, duty_cycle);
}

float Wheel::getSpeed()
{
    return this->encoder.getAngularSpeed() * 2 * M_PI * radius_mm / 360;
}

void Wheel::stop()
{
    this->dc_motor.spin(0, 0);
    this->encoder.resetReadings();
}
