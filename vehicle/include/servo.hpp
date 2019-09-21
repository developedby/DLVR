#ifndef servo_h_
#define servo_h_

class Servo
{
    private:
        Servo servo;
        int pwm_pin;
    public:
        int const min_pos = 30;
        int const max_pos = 130;
        double const crnt_pos;
        Servo(int const pin);
        void goPos(double const pos);
        void release();
}

#endif //servo_h_
