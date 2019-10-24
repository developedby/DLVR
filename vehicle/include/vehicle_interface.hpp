#ifndef vehicle_interface_hpp_
#define vehicle_interface_hpp_

#include "vision.hpp"
#include "movement.hpp"
#include "radio_communication.hpp"
#include "delivery_box.hpp"
#include "intelligence.hpp"
class VehicleInterface
{
    private:
        Vision vision;
        Movement movement;
        RadioCommunication communication;
        DeliveryBox box;
        Intelligence intelligence;
    public:
        VehicleInterface(){}
        virtual void start();
};

#endif
