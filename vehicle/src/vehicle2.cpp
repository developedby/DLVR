#include "vehicle.hpp"

Vehicle::Vehicle() : VehicleInterface()
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
