#ifndef intelligence_hpp_
#define intelligence_hpp_
//#include "vehicle.hpp"

class Vehicle;
class Intelligence
{
    private:
        Vehicle *vehicle;
    public:
        Intelligence(Vehicle *_vehicle);
};
#endif
