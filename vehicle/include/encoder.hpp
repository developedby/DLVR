#ifndef encoder_hpp_
#define encoder_hpp_
#include <cstdint>

class Encoder
{
    public:
        Encoder(int encoder_num);
        float getAngularSpeed();
        void registerMeasurement(int const level, uint32_t const tick);
        void registerStopped();
        int static const num_holes = 20;
        int static const n_measures = 10;
        float measures_us[n_measures];
    private:
        static int instances;
        int const pin_read;
        uint32_t last_measure;
        uint32_t static const max_measure_interval = 1000000;
        float space_weight[n_measures] = {};
        int counter = 0;
        void createSpatialWeights();
        float const sigma_space;
        float const sigma_time;
};

void callRegisterMeasurement(int const gpio, int const level, uint32_t const tick, void * encoder);
void callRegisterStopped(void * encoder_);

#endif //encoder_h_
