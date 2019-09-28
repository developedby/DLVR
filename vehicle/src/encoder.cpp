#include "encoder.hpp"
#include <iostream>
#include <cstdint>
#include <cmath>
#include <pigpio.h>

int Encoder::instances = 0;

Encoder::Encoder(int const pin) : pin_read(pin)
{
    instances++;
    createWeights(1.3);
    gpioSetMode(pin_read, PI_INPUT);
    gpioSetAlertFuncEx(pin_read, callRegisterReading, this);
    gpioSetTimerFuncEx(instances, max_reading_interval/4000, callRegisterStopped, this);
}

float Encoder::getAngularSpeed()
{
    int avg_time = 0;
    int const counter_ = counter;
    for(int i=0; i < num_readings; i++)
    {
        avg_time += weights[i] * readings_us[(i+counter_) % num_readings];
    }
    return (360.0/num_holes) / (avg_time/1000000.0);
}

void Encoder::registerReading(int const level, uint32_t const tick)
{
    readings_us[counter] = tick - last_reading;
    last_reading = tick;
    std::cout << counter << " " << readings_us[counter] << std::endl;
    if (counter >= num_readings-1)
        counter = 0;
    else
        counter++;
}

void Encoder::createWeights(float const sigma)
{
    float sum_weights = 0;
    for (int i=0; i < num_readings; i++)
    {
        weights[i] = exp(-i*i/(2*sigma*sigma));
        sum_weights += weights[i];
    }
    for (int i=0; i < num_readings; i++)
    {
        weights[i] /= sum_weights;
    }
}

void Encoder::registerStopped()
{
    uint32_t const crnt_tick = gpioTick();
    if (crnt_tick - last_reading > max_reading_interval)
    {
        readings_us[counter] = crnt_tick - last_reading;
        last_reading = crnt_tick;
        if (counter < num_readings)
            counter++;
        else
            counter = 0;
    }
}

void callRegisterReading(int const gpio, int const level, uint32_t const tick, void * encoder_)
{
    Encoder * encoder = static_cast<Encoder*>(encoder_);
    encoder->registerReading(level, tick);
}

void callRegisterStopped(void * encoder_)
{
    Encoder * encoder = static_cast<Encoder *>(encoder_);
    encoder->registerStopped();
}
