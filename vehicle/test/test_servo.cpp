#include <wiringPi.h>
#include <iostream>
#include "servo.hpp"

int const servo_pin = 1;

int main (void)
{
    std::cout << "Jogando servo pro minimo, pro maximo e depois soltando" << std::endl;
    wiringPiSetup();
    Servo servo(servo_pin);
    
    while(1)
    {
        servo.goPos(0);
        std::cout << "Posição minima" << std::endl;
        delay(1000);
        servo.goPos(1);
        std::cout << "Posição máxima" << std::endl;
        delay(1000);
    }
    return 0;
}
