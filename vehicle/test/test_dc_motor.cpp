#include <wiringPi.h>
#include <iostream>
#include "dc_motor.hpp"

int const pwm_pin1 = 1;
bool const hw_pwm1 = true;
int const fwd_pin1 = 28;
int const bkwd_pin1 = 29;

int const pwm_pin2 = 26;
bool const hw_pwm2 = true;
int const fwd_pin2 = 11;
int const bkwd_pin2 = 0;

int main (void)
{
    std::cout << "Testando giro do motor..." << std::endl;
    wiringPiSetup();
    DCMotor motor1(fwd_pin1, bkwd_pin1, pwm_pin1, hw_pwm1);
    DCMotor motor2(fwd_pin2, bkwd_pin2, pwm_pin2, hw_pwm2);
    
    //motor1.spin(0, 0);
    //motor2.spin(0, 0);
    delay(2000);
    while (true)
    {
        for (float i = 1; i >= 0; i-= 0.1)
        {
            std::cout << "Girando pra frente " << i << std::endl;
            motor1.spin(1, i);
            motor2.spin(1, i);
            delay(1000);
            std::cout << "Girando pra tras " << i << std::endl;
            motor1.spin(-1, i);
            motor2.spin(-1, i);
            delay(1000);
        }
    }
    std::cout << "Parando motor" << std::endl;
    motor1.spin(0, 0);
    motor2.spin(0, 0);
    delay(1000);
    return 0;
}
