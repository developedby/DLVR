#include "item_detector.hpp"
#include <pigpio.h>

ItemDetector::ItemDetector(int const pin1, int const pin2) : pin_read1(pin1), pin_read2(pin2)
{
    gpioSetMode(pin_read1, PI_INPUT);
    gpioSetMode(pin_read2, PI_INPUT);
}

bool ItemDetector::hasItem()
{
    return !gpioRead(pin_read1) || !gpioRead(pin_read2);
}
