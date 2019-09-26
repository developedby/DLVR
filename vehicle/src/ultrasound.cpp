#include "ultrasound.hpp"
#include <wiringPi.h>

Ultrasound * the_ultrasound;

Ultrasound::Ultrasound(int const pin_trigger_, int const pin_echo_) : pin_trigger(pin_trigger_), pin_echo(pin_echo_)
{
    the_ultrasound = this;
    pinMode(pin_trigger, OUTPUT);
    pinMode(pin_echo, INPUT);
    digitalWrite(pin_trigger, LOW);
    wiringPiISR(pin_echo, INT_EDGE_BOTH, &callRegisterEcho);
    pollDistance();
}

void Ultrasound::pollDistance()
{
    last_poll = millis();
    digitalWrite(pin_trigger, HIGH);
    delayMicroseconds(20);
    digitalWrite(pin_trigger, LOW);
}

void Ultrasound::registerEcho()
{
    static long start_time, end_time;
    if (digitalRead(pin_echo) == HIGH)
    {
        start_time = micros();
    }
    else
    {
        end_time = micros();
        last_distance_cm = (end_time - start_time) * 0.00017;  // 340m/s * 100cm/m * 1/1000000s/us * 1/2 (ida e volta)
        long time_since_poll = millis() - last_poll;
        if (time_since_poll < max_poll_rate_ms)
            delay(max_poll_rate_ms - time_since_poll);
        pollDistance();
    }
}

double Ultrasound::getDistance()
{
    return last_distance_cm;
}

void callRegisterEcho()
{
    the_ultrasound->registerEcho();
}
