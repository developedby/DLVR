#ifndef movement_hpp_
#define movement_hpp_

#include "pidz.hpp"
#include "wheel.hpp" 

class Movement {
    private:
        float wheel_distance;
        PIDZ left_pid;
        PIDZ right_pid;
        Wheel left_wheel;
        Wheel right_wheel;
        float lr;
        float rr;
        float balance;
        int l_dir;
        int r_dir;
        bool iwflag;
        
        float limit(float vmin, float v, float vmax);

    public:
        Movement(float wheel_distance, float l_Kp, float l_Ki, float l_Kd, float r_Kp, float r_Ki, float r_Kd, float T,
                 int const l_enc_pin, int const l_motor_fwd_pin, int const l_motor_bkwd_pin, int const l_motor_pwm_pin,
                 int const r_enc_pin, int const r_motor_fwd_pin, int const r_motor_bkwd_pin, int const r_motor_pwm_pin);
        void tick(void);
        void turn(float degrees);
        void goStraight(int direction, float speed);
        void goCurve(int direction, float curvature);
        float getBalance(void);
        float setBalance(float balance);
};

#endif
