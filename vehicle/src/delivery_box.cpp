#include "delivery_box.hpp"
#include "constants.hpp"

DeliveryBox::DeliveryBox() {}

void DeliveryBox::lock()
{
    lock_obj.lock();
    box_closed = true;
}

void  DeliveryBox::unlock()
{
    lock_obj.unlock();
    box_closed = false;
}

bool DeliveryBox::hasItem()
{
    return detector.hasItem();
}

bool DeliveryBox::isBoxClose()
{
    return box_closed;
}

