#ifndef lock_h_
#define lock_h_
#include "servo.hpp"

class Lock
{
    private:
        Servo servo;
        double const lock_pos = 0;
        double const unlock_pos = 1;
    public:
        Lock(int const servo_pin);
        void lock();
        void unlock();
}

#endif lock_h_
