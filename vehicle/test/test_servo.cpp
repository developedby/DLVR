#include <wiringPi.h>
#include <iostream>
#include "servo.hpp"

int const servo_pin = 0;

int main (void)
{
    std::cout << "";
    wiringPiSetup();
    Servo servo(servo_pin);
    
    servo.goPos(servo.min_pos);
    delay(1000);
    servo.goPos(servo.max_pos);
    delay(1000);
    return 0;
}
