#include <wiringPi.h>
#include <iostream>
#include "ultrasound.hpp"

int const trigger_pin = 0;
int const echo_pin = 2;

int main (void)
{
    std::cout << "Testando o ultrasom amostrando 10 vezes em 10 segundos" << std::endl;
    wiringPiSetup();
    Ultrasound ultrasound(trigger_pin, echo_pin);
    
    delay(1000);
    for (int i=0; i < 10; i++)
    {
        std::cout << ultrasound.getDistance() << std::endl;
        delay(1000);
    }
    return 0;
}
