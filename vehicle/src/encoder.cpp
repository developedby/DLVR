#include "encoder.hpp"
#include <iostream>
#include <cstdint>
#include <cmath>
#include <pigpio.h>
#include "constants.hpp"

int Encoder::instances = 0;

Encoder::Encoder(int encoder_num) : last_measure(gpioTick()), sigma_space(2.0), sigma_time(2.0)
{
    if(encoder_num == 0)
    {
        pin_read = constants::left_encoder_pin;
    }
    else
    {
        pin_read = constants::right_encoder_pin;
    }
    this->instances++;
    this->createSpatialWeights();
    for (int i=0; i < n_measures; i++)
        this->measures_us[i] = this->max_measure_interval;
    gpioSetMode(this->pin_read, PI_INPUT);
    gpioSetAlertFuncEx(this->pin_read, callRegisterMeasurement, this);
    gpioSetTimerFuncEx(this->instances, this->max_measure_interval/(4*1000), callRegisterStopped, this);
}

// Returns the measured angular speed using a gaussian over the last 'n' measurements
float Encoder::getAngularSpeed()
{    
    // Takes a local copy of the speed measurements
    int const counter_ = counter;
    //std::cout << "counter=" << counter_ << std::endl;
    float measures_cpy[this->n_measures];
    for (int i=0; i < n_measures; i++)
    {
        measures_cpy[n_measures-1-i] = this->measures_us[(counter_ + i) % this->n_measures];
        //std::cout << "measure cpy " << measures_cpy[n_measures-1-i] << " index=" << n_measures-1-i << std::endl;
    }
    
    // Calculates tsec_agohe temporal weights
    float time_weight[this->n_measures];
    float sec_ago = (gpioTick() - this->last_measure) / 1000000.0;
    for (int i=0; i < this->n_measures; i++)
    {
        time_weight[i] = exp(-sec_ago*sec_ago / (this->sigma_time*this->sigma_time));
        sec_ago += measures_cpy[i] / 1000000.0;
        //std::cout << "time_weight " << time_weight[i] << " sec_ago " << sec_ago << std::endl;
    }
    
    // Multiplies both weights
    float total_weight[this->n_measures];
    float weight_sum = 0;
    for (int i=0; i < this->n_measures; i++)
    {
        total_weight[i] = this->space_weight[i] * time_weight[i];
        weight_sum += total_weight[i];
    }
    
    // Normalizes the weights
    for (int i=0; i < this->n_measures; i++)
    {
        total_weight[i] /= weight_sum;
        //std::cout << "total_weight[i] " << total_weight[i] << std::endl;
    }
    
    // Takes a weighted average of the measurements with double gaussian weights (space and time)
    float avg_time = 0;
    for (int i=0; i < this->n_measures; i++)
    {
        avg_time += total_weight[i] * measures_cpy[i];
    }
    avg_time /= 1000000.0;
    //std::cout << "avg_time " << avg_time << std::endl;
    return (360.0/this->num_holes) / avg_time;
}

void Encoder::registerMeasurement(int const level, uint32_t const tick)
{
    this->measures_us[counter] = tick - this->last_measure;
    this->last_measure = tick;
    //std::cout << counter << " " << readings_us[counter] << std::endl;
    if (this->counter >= this->n_measures-1)
        this->counter = 0;
    else
        this->counter++;
}

// Creates gaussian spatial weights (relative to the order in which they were taken)
// The idea is to give more weight to more recent (in number of measures) measurementes of speed
void Encoder::createSpatialWeights()
{
    for (int i=0; i < this->n_measures; i++)
    {
        this->space_weight[i] = exp(-i*i/(this->sigma_space*this->sigma_space));
    }
}

void Encoder::registerStopped()
{
    uint32_t const crnt_tick = gpioTick();
    if (crnt_tick - this->last_measure > this->max_measure_interval)
    {
        this->measures_us[this->counter] = crnt_tick - this->last_measure;
        this->last_measure = crnt_tick;
        if (this->counter < this->n_measures)
            this->counter++;
        else
            this->counter = 0;
    }
}

void callRegisterMeasurement(int const gpio, int const level, uint32_t const tick, void * encoder_)
{
    Encoder * encoder = static_cast<Encoder*>(encoder_);
    encoder->registerMeasurement(level, tick);
}

void callRegisterStopped(void * encoder_)
{
    Encoder * encoder = static_cast<Encoder *>(encoder_);
    encoder->registerStopped();
}
