#ifndef vehicle_hpp_
#define vehicle_hpp_

//#include "vision.hpp"
//#include "movement.hpp"
#include "radio_communication.hpp"
#include "delivery_box.hpp"
#include "intelligence.hpp"
class Vehicle
{
    private:
        //Vision vision;
        //Movement movement;
        RadioCommunication communication;
        DeliveryBox box;
        Intelligence intelligence;
    public:
        Vehicle();
};

#endif
