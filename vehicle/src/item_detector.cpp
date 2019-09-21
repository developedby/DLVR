#include "item_detector.hpp"
#include <wiringPi.h>

ItemDetector::ItemDetector(int pin) : pin_read(pin)
{
    pinMode(pin_read, INPUT);
}

bool ItemDetector::hasItem()
{
    return (bool)digitalRead(pin_read);
}
