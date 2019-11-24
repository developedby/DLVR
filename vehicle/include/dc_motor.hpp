#ifndef dc_motor_hpp_
#define dc_motor_hpp_

#include "constants.hpp"

class DCMotor
{
    private:
        int pin_fwd;
        int pin_bkwd;
        int pin_pwm;
    public:
        DCMotor(const consts::WheelType wheel_type);
        void spin(int const direction, double const duty_cycle);
};

#endif //dc_motor_h
