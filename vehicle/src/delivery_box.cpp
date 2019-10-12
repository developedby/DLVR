#include "delivery_box.hpp"
#include "constants.hpp"

DeliveryBox::DeliveryBox() {}

void DeliveryBox::lock()
{
    lock_obj.lock();
}

void  DeliveryBox::unlock()
{
    lock_obj.unlock();
}

bool DeliveryBox::hasItem()
{
    return detector.hasItem();
}

