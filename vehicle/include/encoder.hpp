#ifndef encoder_hpp_
#define encoder_hpp_
#include <cstdint>

class Encoder
{
    public:
        int static constexpr num_holes = 20;
        int static constexpr n_measures = 10;
        int ticks;
        float measures_us[n_measures];
        Encoder(int encoder_num);
        float getAngularSpeed();
        void registerMeasurement(int const level, uint32_t const tick);
        void registerStopped();
        void resetReadings();
    private:
        static int instances;
        int pin_read;
        uint32_t last_measure;
        uint32_t static constexpr max_measure_interval = 1000000;
        uint32_t static constexpr min_measure_interval = 10000;  // Time to spin one hole at 1m/s
        float space_weight[n_measures] = {};
        int counter = 0;
        void createSpatialWeights();
        float const sigma_space;
        float const sigma_time;
};

void callRegisterMeasurement(int const gpio, int const level, uint32_t const tick, void * encoder);
void callRegisterStopped(void * encoder_);

#endif //encoder_h_
