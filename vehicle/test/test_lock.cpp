#include <wiringPi.h>
#include <iostream>
#include "lock.hpp"

int const lock_pin = 0;

int main (void)
{
    std::cout << "";
    wiringPiSetup();
    Lock lock(lock_pin);

    lock.lock();
    delay(1000);
    lock.unlock();
    delay(1000);
    return 0;
}
