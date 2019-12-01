#include "vehicle.hpp"

Vehicle::Vehicle() : intelligence(this)
{
}

void Vehicle::start()
{
	movement.stop();
	box.lock();
	intelligence.mainLoop();
}
