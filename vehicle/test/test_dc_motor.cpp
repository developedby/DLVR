#include <iostream>
#include <pigpio.h>
#include "dc_motor.hpp"

int const pwm_pin1 = 18;
int const fwd_pin1 = 20;
int const bkwd_pin1 = 21;

int const pwm_pin2 = 12;
int const fwd_pin2 = 7;
int const bkwd_pin2 = 17;

int main (void)
{
    std::cout << "Testando giro do motor..." << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    DCMotor motor1(fwd_pin1, bkwd_pin1, pwm_pin1);
    DCMotor motor2(fwd_pin2, bkwd_pin2, pwm_pin2);
    
    //motor1.spin(0, 0);
    //motor2.spin(0, 0);
    gpioDelay(2000000);
    while (true)
    {
        for (float i = 1; i >= 0; i-= 0.1)
        {
            std::cout << "Girando pra frente " << i << std::endl;
            motor1.spin(1, i);
            motor2.spin(1, i);
            gpioDelay(1000000);
            std::cout << "Girando pra tras " << i << std::endl;
            motor1.spin(-1, i);
            motor2.spin(-1, i);
            gpioDelay(1000000);
        }
    }
    std::cout << "Parando motor" << std::endl;
    motor1.spin(0, 0);
    motor2.spin(0, 0);
    gpioDelay(1000000);
    gpioTerminate();
    return 0;
}
