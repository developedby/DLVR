#ifndef delivery_box_hpp_
#define delivery_box_hpp_

#include "item_detector.hpp"
#include "lock.hpp"

class DeliveryBox
{
    private:
        ItemDetector detector;
        Lock lock_obj;
    public:
        DeliveryBox();
        void lock();
        void unlock();
        bool hasItem();     
};

#endif
