#include <wiringPi.h>
#include <iostream>
#include "servo.hpp"

int const servo_pin = 1;

int main (void)
{
    std::cout << "Jogando servo pro minimo, pro maximo e depois soltando" << std::endl;
    wiringPiSetup();
    Servo servo(servo_pin);
    
    servo.goPos(servo.min_pos);
    std::cout << "Posição minima" << std::endl;
    delay(1000);
    servo.goPos(servo.max_pos);
    std::cout << "Posição máxima" << std::endl;
    delay(1000);
    std::cout << "Servo solto" << std::endl;
    servo.release();
    delay(5000);
    return 0;
}
