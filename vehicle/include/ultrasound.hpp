#ifndef ultrasound_h_
#define ultrasound_h_

class Ultrasound
{
    private:
        int pin_trigger;
        int pin_echo;
        double last_distance_cm;
        int max_poll_rate_ms = 100;
        void pollDistance();
        void registerEcho();
        long trigger_time;
        long last_poll
    public:
        Ultrasound(int const pin_trigger_, int const pin_echo_);
        double getDistance();
}

#endif //ultrasond_h_