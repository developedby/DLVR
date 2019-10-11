#include "wheel.hpp"
#include <iostream>
#include <cstdint>
#include <cmath>
#include <pigpio.h>


Wheel::Wheel(int wheel_num) : encoder(wheel_num), dc_motor(wheel_num) {}

void Wheel::spin(int const direction, float const duty_cycle)
{
    //std::cout << "Girando direcao " << direction << " dc " << duty_cycle << std::endl;
    dc_motor.spin(direction, duty_cycle);
}

float Wheel::getSpeed()
{
    return encoder.getAngularSpeed() * 2 * M_PI * radius_mm / 360;
}
