#ifndef pidz_hpp_
#define pidz_hpp_

#include <limits>

class PIDZ {
    private:
        float nn[3];
        float dd[2];
        float  x[3];
        float  y[2];
        float output;
        float T;
        float omin, omax;
    public:
        PIDZ(float Kp, float Ki, float Kd, float T);
        PIDZ(float Kp, float Ki, float Kd, float T, float min, float max);
        void tune(float Kp, float Ki, float Kd);
        float push_error(float e);
        float push_error(float rf, float fb);
        float peek_output(void);
};

#endif