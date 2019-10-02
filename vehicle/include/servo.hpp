#ifndef servo_h_
#define servo_h_

class Servo
{
    private:
        int pwm_pin;
    public:
        double crnt_pos;
        bool active;
        Servo(int const pin);
        void goPos(double const pos);
        void release();
};


#endif //servo_h_
