#ifndef wheel_hpp_
#define wheel_hpp_
#include <cstdint>
#include "encoder.hpp"
#include "dc_motor.hpp"

class Wheel
{
    private:
        Encoder encoder;
        DCMotor dc_motor;
        static constexpr float radius_mm = 33.0;
        int last_encoder_ticks;
    public:
        Wheel(int wheel_num);
        void spin(int const direction, float const duty_cycle);
        float getSpeed();
        void stop();
        float cmMovedSinceLastCall();
};

#endif //encoder_h_
