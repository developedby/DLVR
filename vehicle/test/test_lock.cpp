#include <wiringPi.h>
#include <iostream>
#include "lock.hpp"

int const lock_pin = 1;

int main (void)
{
    std::cout << "Travando..." << std::endl;
    wiringPiSetup();
    Lock lock(lock_pin);

    delay(1000);
    lock.lock();
    std::cout << "Travado" << std::endl;
    delay(2000);
    //std::cout << "Destravado" << std::endl;
    //lock.unlock();
    delay(1000);
    return 0;
}
