#ifndef __PIDZ__
#define __PIDZ__

class PIDZ {
    private:
        float nn[3];
        float dd[2];
        float x[3];
        float y[2];
        float output;
        float T;
    public:
        PIDZ(float Kp, float Ki, float Kd, float T);
        float push_error(float e);
        float peek_output(void);
};

#endif