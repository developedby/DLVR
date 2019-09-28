#include <iostream>
#include <pigpio.h>
#include "ultrasound.hpp"

int const trigger_pin = 27;
int const echo_pin = 22;

int main (void)
{
    std::cout << "Testando o ultrasom amostrando 10 vezes em 10 segundos" << std::endl;
    gpioInitialise();
    Ultrasound ultrasound(trigger_pin, echo_pin);
    gpioDelay(1000000);
    
    float dist;
    for (int i=0; ; i++)
    {
        dist = ultrasound.getDistance();
        if (dist > 300)
            std::cout << "Não detectou nada" << std::endl;
        else
            std::cout << "Objeto a " << dist << "cm" << std::endl;
        gpioDelay(1100000);
    }
    gpioTerminate();
    return 0;
}
