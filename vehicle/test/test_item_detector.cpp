#include <iostream>
#include <pigpio.h>
#include "item_detector.hpp"

int const pin1 = 2;
int const pin2 = 5;

int main (void)
{
    std::cout << "Testando o detector de itens amostrando 20 vezes em 20 segundos" << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    ItemDetector detector(pin1, pin2);
    gpioDelay(2000000);
    
    for (int i=0; i < 20; i++)
    {
        std::cout << (bool)(detector.hasItem()) << std::endl;
        gpioDelay(1000000);
    }
    return 0;
}
