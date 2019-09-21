#include <wiringPi.h>
#include <iostream>
#include "lock.hpp"

int const lock_pin = 1;

int main (void)
{
    std::cout << "Travando e depois destravando..." << std::endl;
    wiringPiSetup();
    Lock lock(lock_pin);

    lock.lock();
    std::cout << "Travado" << std::endl;
    delay(1000);
    std::cout << "Destravado" << std::endl;
    lock.unlock();
    delay(1000);
    return 0;
}
