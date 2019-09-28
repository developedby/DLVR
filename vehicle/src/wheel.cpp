#include "wheel.hpp"
#include <iostream>
#include <cstdint>
#include <cmath>
#include <pigpio.h>


Wheel::Wheel(int const enc_pin, int const motor_fwd_pin, int const motor_bkwd_pin, int const motor_pwm_pin) : encoder(enc_pin), dc_motor(motor_fwd_pin, motor_bkwd_pin, motor_pwm_pin)
{
}

void Wheel::spin(int const direction, float const duty_cycle)
{
    //std::cout << "Girando direcao " << direction << " dc " << duty_cycle << std::endl;
    dc_motor.spin(direction, duty_cycle);
}

float Wheel::getSpeed()
{
    return encoder.getAngularSpeed() * 2 * M_PI * radius_mm / 360;
}
