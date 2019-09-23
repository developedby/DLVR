#include "dc_motor.hpp"
#include <cassert>
#include <iostream>
#include <wiringPi.h>
#include <softPwm.h>

DCMotor::DCMotor(int const pin_fwd_, int const pin_bkwd_, int const pin_pwm_, bool const hw_pwm_) : pin_fwd(pin_fwd_), pin_bkwd(pin_bkwd_), pin_pwm(pin_pwm_), hw_pwm(hw_pwm_)
{
	if (hw_pwm)
	{
		assert(pin_pwm == 1 || pin_pwm == 26 || pin_pwm == 23 || pin_pwm == 24);
		pinMode(pin_pwm, PWM_OUTPUT);
		pwmSetMode(PWM_MODE_MS);
		pwmSetClock(384);  // 50kHz (tick de 20us)
		pwmSetRange(1000);  // Pulsos de 20us a 20ms
		delay(500);
	}
	else
	{
		softPwmCreate(pin_pwm, 0, 100);
	}
	pinMode(pin_fwd, OUTPUT);
	pinMode(pin_bkwd, OUTPUT);
}

void DCMotor::spin(int const direction, double const duty_cycle)
{
	if (direction == 0)
	{
		std::cout << "Parado\n";
		if (hw_pwm)
			pwmWrite(pin_pwm, 1000);
		else
			softPwmWrite(pin_pwm, 100);
		digitalWrite(pin_fwd, LOW);
		digitalWrite(pin_bkwd, LOW);
	}
	else
	{
		if (direction > 0)
		{
			std::cout << "Frente\n";
			digitalWrite(pin_fwd, HIGH);
			digitalWrite(pin_bkwd, LOW);
		}
		else if (direction < 0)
		{
			std::cout << "Tras\n";
			digitalWrite(pin_fwd, LOW);
			digitalWrite(pin_bkwd, HIGH);
		}
		if (hw_pwm)
			pwmWrite(pin_pwm, (int)((1-duty_cycle)*1000));
		else
			softPwmWrite(pin_pwm, (int)((1-duty_cycle)*100));
	}
}
