#ifndef intelligence_hpp_
#define intelligence_hpp_
#include "vehicle.hpp"
class Intelligence
{
    private:
        Vehicle *vehicle;
    public:
        Intelligence(Vehicle *_vehicle);
        void mainLoop();
        bool hasCommand();
        void followTheRoad();
        void avoidObstacle();
        void decodeCommand();
        void sendFeedback();
};
#endif
