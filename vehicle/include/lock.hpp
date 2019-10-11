#ifndef lock_hpp_
#define lock_hpp_
#include "servo.hpp"

class Lock
{
    private:
        Servo servo;
        double const lock_pos = 0.5;
        double const unlock_pos = 0.0;
    public:
        Lock(int const servo_pin);
        void lock();
        void unlock();
};

#endif //lock_h_
