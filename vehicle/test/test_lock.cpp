#include <iostream>
#include <pigpio.h>
#include "lock.hpp"

int const lock_pin = 13;

int main (void)
{
    std::cout << "Travando..." << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Lock lock(lock_pin);

    gpioDelay(1000000);
    lock.lock();
    std::cout << "Travado" << std::endl;
    gpioDelay(2000000);
    std::cout << "Destravado" << std::endl;
    lock.unlock();
    gpioDelay(1000000);
    return 0;
}
