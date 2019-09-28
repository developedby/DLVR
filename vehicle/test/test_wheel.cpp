#include <iostream>
#include <pigpio.h>
#include "wheel.hpp"

int const l_enc_pin = 26;
int const l_motor_fwd_pin = 21;
int const l_motor_bkwd_pin = 20;
int const l_motor_pwm_pin = 18;

int const r_enc_pin = 3;
int const r_motor_fwd_pin = 17;
int const r_motor_bkwd_pin = 7;
int const r_motor_pwm_pin = 12;


int main (void)
{
    std::cout << "Lendo velocidade do encoder 1 vez por segundo" << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Wheel l_wheel(l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin);
    Wheel r_wheel(r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin);
    gpioDelay(2000000);
    
    float speed_l, speed_r;
    while (true)
    {
        for (float i = 1; i >= 0; i-= 0.1)
        {
            l_wheel.spin(1, i);
            r_wheel.spin(1, i);
            speed_l = l_wheel.getSpeed();
            speed_r = r_wheel.getSpeed();
            std::cout << "Esquerda: " << speed_l << " mm/s" << std::endl;
            std::cout << "Direita: " << speed_r << " mm/s" << std::endl << std::endl;
            gpioDelay(1000000);
        }
    }
    gpioTerminate();
    return 0;
}
