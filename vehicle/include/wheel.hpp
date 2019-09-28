#ifndef wheel_h_
#define wheel_h_
#include <cstdint>
#include "encoder.hpp"
#include "dc_motor.hpp"

class Wheel
{
    private:
        Encoder encoder;
        DCMotor dc_motor;
        static constexpr float radius_mm = 33.0;
    public:
        Wheel(int const enc_pin, int const motor_fwd_pin, int const motor_bkwd_pin, int const motor_pwm_pin);
        void spin(int const direction, float const duty_cycle);
        float getSpeed();
};

#endif //encoder_h_
