#ifndef __PIDZ__
#define __PIDZ__

class PIDZ {
    private:
        float a[3];
        float x[3];
        float y;
        int insert;
    public:
        PIDZ(float Kp, float Ki, float Kd, float T);
        float push_error(float e);
        float peek_output(void);
}

#endif