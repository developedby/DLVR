#ifndef pidz_hpp_
#define pidz_hpp_

#include "constants.hpp"

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
        PIDZ(const consts::WheelType wheel_type);
        void tune(const float Kp, const float Ki, const float Kd);
        float push_error(const float e);
        float push_error(const float rf, const float fb);
        float peek_output(void);
};

#endif
