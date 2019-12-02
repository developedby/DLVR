#ifndef movement_hpp_
#define movement_hpp_

#include "pidz.hpp"
#include "wheel.hpp"

class Movement {
    private:
        PIDZ left_pid;
        PIDZ right_pid;
        Wheel left_wheel;
        Wheel right_wheel;
        float lr;
        float rr;
        float balance;
        float lb;
        float rb;
        int l_dir;
        int r_dir;
        int turn_ticks;
        bool initial_wheel_flag;
        float lm_speed;
        float rm_speed;
        float required_speed;
        bool moving_left;
        bool moving_right;
        int last_movement_dir;
    public:
        Movement();
        void tick(void);
        void turn(const float degrees);
        void turnOneWheel(const consts::WheelType wheel_type, const int direction, const float mm);
        void goStraight(const int direction, const float speed);
        float goStraightMm(const int direction, const float mm, const float speed);
        void goCurve(const int direction, const float curvature);
        void stop();
        float getBalance(void);
        float setBalance(const float balance);
        bool isTurning(void);
        void calculateSpeed();
};

inline float signum(float num)
{
        return (0 < num) - (num < 0);
}

#endif
