#include "servo.hpp"
#include <cassert>
#include <pigpio.h>

Servo::Servo(int const pin) : pwm_pin(pin)
{
    assert(pwm_pin == 12 || pwm_pin == 13 || pwm_pin == 18 || pwm_pin == 19);
    if (pwm_pin == 12 || pwm_pin == 13)
        gpioSetMode(pwm_pin, PI_ALT0);
    else
        gpioSetMode(pwm_pin, PI_ALT5);
    active = false;
}
// Manda o servo pra uma posição e trava ele lá
// pos é um valor de 0 (posição minima) a 1 (posição máxima)
void Servo::goPos(double const pos)
{
    gpioServo(pwm_pin, (2500-500)*pos + 500);
    crnt_pos = pos;
    active = true;
    gpioDelay(500000);
    release();
}

// Solta o servo. Na pratica, manda pwm com duty cycle 0
void Servo::release()
{
    gpioServo(pwm_pin, 0);
    active = false;
}
