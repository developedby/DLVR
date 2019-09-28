#ifndef encoder_h_
#define encoder_h_
#include <cstdint>

class Encoder
{
    private:
        static int instances;
        int pin_read;
        int static const num_holes = 20;
        int static const num_readings = 10;
        float readings_us[num_readings] = {};
        uint32_t last_reading = 0;
        uint32_t static const max_reading_interval = 3000000;
        float weights[num_readings] = {};
        int counter = 0;
        void createWeights (float const sigma);
    public:
        Encoder(int const pin);
        float getAngularSpeed();
        void registerReading(int const level, uint32_t const tick);
        void registerStopped();
};

void callRegisterReading(int const gpio, int const level, uint32_t const tick, void * encoder);
void callRegisterStopped(void * encoder_);

#endif //encoder_h_
