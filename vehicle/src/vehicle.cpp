#include "vehicle.hpp"

Vehicle::Vehicle() : intelligence(this)
{
}

void Vehicle::start()
{
	intelligence.mainLoop();
}

int main()
{
	Vehicle vehicle = Vehicle();
	vehicle.start();
}
