#include <wiringPi.h>
#include <iostream>
#include "item_detector.hpp"

int const detector_pin = 0;

int main (void)
{
    std::cout << "Testando o detector de itens amostrando 10 vezes em 10 segundos" << std::endl;
    wiringPiSetup();
    ItemDetector detector(detector_pin);
    
    for (int i=0; i < 10; i++)
    {
        cout << detector.hasItem() << std::endl;
        delay(1000);
    }
    return 0;
}
