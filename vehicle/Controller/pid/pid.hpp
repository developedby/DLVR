#ifndef __PID__
#define __PID__

#include <cmath>

class PID {
    public:
        float Kp, Ki, Kd, T;
        float integrate;
        float last_error;
        float y;
        
        float minv, maxv;
        
    public:
        PID(float Kp, float Ki, float Kd, float T);
        PID(float Kp, float Ki, float Kd, float T, float min, float max);
        void tunning(float Kp, float Ki, float Kd);
        float push_error(float e);
        float push_error(float r, float y);
        float push_setpoint(float sp);
        float peek_output();
};

#endif
