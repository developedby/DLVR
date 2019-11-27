#ifndef wheel_hpp_
#define wheel_hpp_
#include <cstdint>
#include "encoder.hpp"
#include "dc_motor.hpp"
#include "constants.hpp"

class Wheel
{
    private:
        DCMotor dc_motor;
    public:
        int last_encoder_ticks;
        Encoder encoder;
        Wheel(const consts::WheelType wheel_type);
        void spin(int const direction, float const duty_cycle);
        float getSpeed();
        void stop();
        float mmMovedSinceLastCall();
};

#endif //encoder_h_
