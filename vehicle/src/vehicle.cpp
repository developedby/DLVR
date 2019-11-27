#include "vehicle.hpp"

Vehicle::Vehicle() : intelligence(this)
{
}

void Vehicle::start()
{
	movement.stop();
	intelligence.mainLoop();
}
