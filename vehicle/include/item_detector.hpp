#ifndef item_detector_h_
#define item_detector_h_

class ItemDetector
{
    private:
        int pin_read1;
        int pin_read2;
    public:
        ItemDetector(int pin1, int pin2);
        bool hasItem();
};

#endif //item_detector_h_
