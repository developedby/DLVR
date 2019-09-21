#ifndef item_detector_h_
#define item_detector_h_

class ItemDetector
{
    private:
        int pin_read;
    public:
        ItemDetector(int pin);
        bool hasItem();
}

#endif //item_detector_h_
