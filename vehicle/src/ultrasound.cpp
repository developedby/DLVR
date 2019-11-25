#include "ultrasound.hpp"
#include <iostream>
#include <cstdint>
#include <pigpio.h>
#include "constants.hpp"

Ultrasound::Ultrasound()
{
    pin_trigger = consts::ultrasound_trigger_pin;
    pin_echo = consts::ultrasound_echo_pin;
    gpioSetMode(pin_trigger, PI_OUTPUT);
    gpioSetMode(pin_echo, PI_INPUT);
    gpioWrite(pin_trigger, 0);
    gpioSetAlertFuncEx(pin_echo, callRegisterEcho, this);
    gpioSetTimerFuncEx(0, poll_rate_ms, callPollDistance, this);
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


void callPollDistance(void *obj)
{
    Ultrasound *the_ultrasound = static_cast<Ultrasound*>(obj);
    the_ultrasound->pollDistance();
}

void callRegisterEcho(int const gpio, int const level, uint32_t const tick, void *obj)
{
    Ultrasound *the_ultrasound = static_cast<Ultrasound*>(obj); 
    the_ultrasound->registerEcho(level, tick);
}
