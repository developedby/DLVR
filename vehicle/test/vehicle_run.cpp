#include <iostream>
#include <pigpio.h>
#include "vehicle.hpp"

int main()
{
	if (gpioInitialise() == PI_INIT_FAILED)
	{
		std::cout << "Erro ao inicializar!" << std::endl;
		exit(PI_INIT_FAILED);
	}
	Vehicle vehicle = Vehicle();
	vehicle.start();
	gpioTerminate();
	return 0;
}
