#ifndef vehicle_hpp_
#define vehicle_hpp_

#include "vehicle_interface.hpp"
class Vehicle:public VehicleInterface
{
    public:
        Vehicle();
        void start();
};

#endif
