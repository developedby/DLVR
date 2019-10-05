#include <iostream>
#include <pigpio.h>
#include "encoder.hpp"

int const pin_left = 4;//26;
int const pin_right = 6;//3;

int main (void)
{
    std::cout << "Lendo velocidade do encoder 1 vez por segundo" << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Encoder encoder_left(pin_left);
    Encoder encoder_right(pin_right);
    
    float speed_l, speed_r;
    while (true)
    {
        for (float i = 1; i >= 0; i-= 0.1)
        {
            for (int j=0; j < Encoder::n_measures; j++)
            {
                 std::cout << "Esquerda " << j << ": " << encoder_left.measures_us[j] << std::endl;
                 std::cout << "Direita " << j << ": " << encoder_right.measures_us[j] << std::endl;
            }
            speed_l = encoder_left.getAngularSpeed();
            speed_r = encoder_right.getAngularSpeed();
            std::cout << "Esquerda: " << speed_l << " deg/s" << std::endl;
            std::cout << "Direita: " << speed_r << " deg/s" << std::endl << std::endl;
            gpioDelay(1000000);
        }
    }
    gpioTerminate();
    return 0;
}
