#include "vehicle.hpp"

Vehicle::Vehicle() : intelligence(this)
{
}

void Vehicle::start()
{
	intelligence.mainLoop();
}
