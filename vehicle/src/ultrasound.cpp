#include "ultrasound.hpp"
#include <wiringPi.h>

Ultrasound::Ultrasound(int const pin_trigger_, int const pin_echo_) : pin_trigger(pin_trigger_), pin_echo(pin_echo_)
{
    pinMode(pin_trigger, OUTPUT);
    pinMode(pin_echo, INPUT);
    digitalWrite(pin_trigger, LOW);
    pollDistance();
}

void Ultrasound::pollDistance()
{
    last_poll = millis();
    digitalWrite(pin_trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_trigger, HIGH);
    trigger_time = micros();
    wiringPiISR(pin_echo, INT_EDGE_BOTH, register_echo);
}

void Ultrasound::registerEcho()
{
    long travel_time = micros() - trigger_time;
    last_distance_cm = travel_time * 0.00017;  // 340m/s * 100cm/m * 1/1000000s/us * 1/2 (ida e volta
    long time_since_poll = millis() - last_poll;
    if (time_since_poll < max_poll_rate_ms)
        delay(max_poll_rate_ms - time_since_poll;
    pollDistance();
}

double Ultrasound::getDistance()
{
    return last_distance_cm;
}
