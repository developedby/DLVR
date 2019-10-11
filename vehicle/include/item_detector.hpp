#ifndef item_detector_hpp_
#define item_detector_hpp_

class ItemDetector
{
    private:
        int pin_read1;
        int pin_read2;
    public:
        ItemDetector();
        bool hasItem();
};

#endif //item_detector_h_
