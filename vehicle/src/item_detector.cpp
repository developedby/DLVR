#include "item_detector.hpp"
#include <pigpio.h>
#include "constants.hpp"
#include <iostream>

ItemDetector::ItemDetector()
{
    pin_read1 = consts::item_detector_pin1;
    pin_read2 = consts::item_detector_pin2;
    gpioSetMode(pin_read1, PI_INPUT);
    gpioSetMode(pin_read2, PI_INPUT);
}

bool ItemDetector::hasItem()
{
    //std::cout << "pino 1: " << gpioRead(pin_read1) << " pino 2 : " << gpioRead(pin_read2) << std::endl;
    return !gpioRead(pin_read1);// || !gpioRead(pin_read2);
}
