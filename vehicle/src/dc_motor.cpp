#include "dc_motor.hpp"
#include <cassert>
#include <iostream>
#include <pigpio.h>
#include <softPwm.h>
#include "constants.hpp"

DCMotor::DCMotor(int motor_num)
{
	if(motor_num == 0)
	{
		pin_fwd = constants::left_motor_pin_fwd;
		pin_bkwd = constants::left_motor_pin_bkwd;
		pin_pwm = constants::left_motor_pin_pwm;
	}
	else
	{
		pin_fwd = constants::right_motor_pin_fwd;
		pin_bkwd = constants::right_motor_pin_bkwd;
		pin_pwm = constants::right_motor_pin_pwm;
	}
	assert(pin_pwm == 18 || pin_pwm == 19 || pin_pwm == 12 || pin_pwm == 13);
	if (pin_pwm == 18 || pin_pwm == 19)
	{
		gpioSetMode(pin_pwm, PI_ALT5);
	}
	else
	{
		gpioSetMode(pin_pwm, PI_ALT0);
	}
	gpioSetPWMfrequency(pin_pwm, 50);
	gpioSetPWMrange(pin_pwm, 1000);
	gpioSetMode(pin_fwd, PI_OUTPUT);
	gpioSetMode(pin_bkwd, PI_OUTPUT);
}

void DCMotor::spin(int const direction, double const duty_cycle)
{
	if (direction == 0)
	{
		//std::cout << "Parado\n";
		gpioPWM(pin_pwm, 1000);
		gpioWrite(pin_fwd, 0);
		gpioWrite(pin_bkwd, 0);
	}
	else
	{
		if (direction > 0)
		{
			//std::cout << "Frente\n";
			gpioWrite(pin_fwd, 1);
			gpioWrite(pin_bkwd, 0);
		}
		else if (direction < 0)
		{
			//std::cout << "Tras\n";
			gpioWrite(pin_fwd, 0);
			gpioWrite(pin_bkwd, 1);
		}
		gpioPWM(pin_pwm, (int)((1-duty_cycle)*1000));
	}
}
