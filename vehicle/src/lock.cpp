#include "lock.hpp"

Lock::Lock() {}

void Lock::lock()
{
    servo.goPos(lock_pos);
}

void Lock::unlock()
{
    servo.goPos(unlock_pos);
}
