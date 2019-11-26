#ifndef ultrasound_h_
#define ultrasound_hpp_
#include <cstdint>

class Ultrasound
{
    private:
        int pin_trigger;
        int pin_echo;
        float last_distance_cm = 0;
        uint32_t start_time = 0;
    public:
        Ultrasound();
        double getDistance();
        void pollDistance();
        void registerEcho(int const level, uint32_t const tick);
        int const poll_rate_ms = 300;
};

void callRegisterEcho(int const gpio, int const level, uint32_t const tick, void *obj);
void callPollDistance(void *obj);

#endif //ultrasond_h_
