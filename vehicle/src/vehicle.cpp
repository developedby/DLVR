#include "vehicle.hpp"
#include <iostream>
Vehicle::Vehicle() : intelligence(this)
{
}

void Vehicle::start()
{
	movement.stop();
	/*box.unlock();
	while(true)
	{
		std::cout << "tem item: " << box.hasItem() << std::endl;
		gpioDelay(100000);
	}*/
	box.lock();
	intelligence.mainLoop();
}
