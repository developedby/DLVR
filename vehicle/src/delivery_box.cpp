#include "delivery_box.hpp"
#include "constants.hpp"

DeliveryBox::DeliveryBox() {}

void DeliveryBox::lock()
{
    lock.lock();
}

void  DeliveryBox::unlock()
{
    lock.unlock();
}

bool DeliveryBox::hasItem()
{
    return detector.hasItem();
}

