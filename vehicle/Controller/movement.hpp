#ifndef __MOVEMENT__
#define __MOVEMENT__

#include "pid.hpp"
#include "wheel.hpp" 

class Movement {
    private:
        float wheel_distance;
        PID left_pid;
        PID right_pid;
        Wheel left_wheel;
        Wheel right_wheel;
        float lr;
        float rr;
        int l_dir;
        int r_dir;
        //static constexpr float l_dc_mmps = 0.003f;
        //static constexpr float r_dc_mmps = 0.004f;
    
    public:
        Movement(float wheel_distance, float l_Kp, float l_Ki, float l_Kd, float r_Kp, float r_Ki, float r_Kd, float T,
                 int const l_enc_pin, int const l_motor_fwd_pin, int const l_motor_bkwd_pin, int const l_motor_pwm_pin,
                 int const r_enc_pin, int const r_motor_fwd_pin, int const r_motor_bkwd_pin, int const r_motor_pwm_pin);
        void tick();
        void turn(float degrees);
        void goStraight(int direction, float speed); //mmps
        void goCurve(int direction, float curvature);
};

#endif
