#include "lock.hpp"

Lock::Lock(int const servo_pin) : servo{servo_pin} {}

void Lock::lock()
{
    servo.goPos(lock_pos);
}

void Lock::unlock()
{
    servo.goPos(unlock_pos);
}
