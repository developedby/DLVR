#include "item_detector.hpp"
#include <pigpio.h>
#include "constants.hpp"

ItemDetector::ItemDetector()
{
    pin_read1 = constants::item_detector_pin1;
    pin_read2 = constants::item_detector_pin2;
    gpioSetMode(pin_read1, PI_INPUT);
    gpioSetMode(pin_read2, PI_INPUT);
}

bool ItemDetector::hasItem()
{
    return !gpioRead(pin_read1) || !gpioRead(pin_read2);
}
