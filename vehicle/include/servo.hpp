#ifndef servo_hpp_
#define servo_hpp_

class Servo
{
    private:
        int pwm_pin;
    public:
        double crnt_pos;
        bool active;
        Servo();
        void goPos(double const pos);
        void release();
};


#endif //servo_h_
