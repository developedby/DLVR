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
        float lb;
        float rb;
        int l_dir;
        int r_dir;
        int turn_ticks;
        bool initial_wheel_flag;
    public:
        Movement();
        void tick(void);
        void turn(float degrees);
        void goStraight(int direction, float speed);
        void goCurve(int direction, float curvature);
        void stop();
        float getBalance(void);
        float setBalance(float balance);
        bool isTurning(void);
};

#endif
