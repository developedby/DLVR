#ifndef __PID_HPP__
#define __PID_HPP__

#include <iostream>

class PID {
    private:
    float Kp;
    float Ki;
    float Kd;
    float T;
    
    float integrate;
    float last_error;
    
    float y;
    
    public:
    PID(float Kp, float Ki, float Kd, float T);
    float push_error(float e);
    float peek_output(void);
    
    
    friend std::ostream& operator<<(std::ostream &strm, const PID &pid);
}

std::ostream& operator<<(std::ostream &strm, const PID &pid);

#endif