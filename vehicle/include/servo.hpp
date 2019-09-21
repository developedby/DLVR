#ifndef servo_h_
#define servo_h_

class Servo
{
    private:
        int pwm_pin;
    public:
        int const min_pos = 30;
        int const max_pos = 130;
        double crnt_pos;
        Servo(int const pin);
        void goPos(double const pos);
        void release();
};

void * scheduleRelease(void * servo_);

#endif //servo_h_
