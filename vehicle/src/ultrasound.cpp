#include "ultrasound.hpp"
#include <iostream>
#include <cstdint>
#include <pigpio.h>

Ultrasound * the_ultrasound;

Ultrasound::Ultrasound(int const pin_trigger_, int const pin_echo_) : pin_trigger(pin_trigger_), pin_echo(pin_echo_)
{
    the_ultrasound = this;
    gpioSetMode(pin_trigger, PI_OUTPUT);
    gpioSetMode(pin_echo, PI_INPUT);
    gpioWrite(pin_trigger, 0);
    gpioSetAlertFunc(pin_echo, callRegisterEcho);
    gpioSetTimerFunc(0, poll_rate_ms, callPollDistance);
}

void Ultrasound::pollDistance()
{
    gpioTrigger(pin_trigger, 10, 1);
}

void Ultrasound::registerEcho(int const level, uint32_t const tick)
{
    uint32_t time_taken;
    if (level == 1)
    {
        start_time = tick;
        //std::cout << "Ping" << std::endl;
    }
    else
    {
        time_taken = tick - start_time;
        last_distance_cm = (float)time_taken / 58.0;  // 340m/s * 100cm/m * 1/1000000s/us * 1/2 (ida e volta)
        //std::cout << "Pong" << std::endl;
    }
}

double Ultrasound::getDistance()
{
    return last_distance_cm;
}


void callPollDistance()
{
    the_ultrasound->pollDistance();
}

void callRegisterEcho(int const gpio, int const level, uint32_t const tick)
{
    the_ultrasound->registerEcho(level, tick);
}
