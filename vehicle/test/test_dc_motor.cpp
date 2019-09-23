#include <wiringPi.h>
#include <iostream>
#include "dc_motor.hpp"

int const pwm_pin1 = 1;
bool const hw_pwm1 = true;
int const fwd_pin1 = 0;
int const bkwd_pin1 = 2;

int const pwm_pin2 = 12;
bool const hw_pwm2 = false;
int const fwd_pin2 = 13;
int const bkwd_pin2 = 14;

int main (void)
{
    std::cout << "Testando giro do motor..." << std::endl;
    wiringPiSetup();
    DCMotor motor1(fwd_pin1, bkwd_pin1, pwm_pin1, hw_pwm1);
    DCMotor motor2(fwd_pin2, bkwd_pin2, pwm_pin2, hw_pwm2);
    
    delay(2000);
    for (int i = 10; i >= 0; i--)
    {
        std::cout << "Girando pra frente " << i*10 << "%" << std::endl;
        motor1.spin(1, (float)(i)/10);
        motor2.spin(1, (float)(i)/10);
        delay(1000);
        std::cout << "Girando pra tras " << i*10 << "%" << std::endl;
        motor1.spin(-1, (float)(i)/10);
        motor2.spin(-1, (float)(i)/10);
        delay(1000);
    }
    std::cout << "Parando motor" << std::endl;
    motor1.spin(0, 0);
    motor2.spin(0, 0);
    delay(1000);
    return 0;
}
