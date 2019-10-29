#include "intelligence.hpp"

Intelligence::Intelligence(Vehicle *_vehicle) : vehicle(_vehicle)
{
    
}

void Intelligence::mainLoop()
{
    while(true)
    {
        if (hasCommand())
        {
            decodeCommand();
        }
        sendFeedback();
    }
    
}

bool Intelligence::hasCommand()
{
    return vehicle->communication.receiveFromRadio();
}

void Intelligence::followTheRoad()
{
}

void Intelligence::avoidObstacle()
{
}

void Intelligence::decodeCommand()
{
}

void Intelligence::sendFeedback()
{
}
