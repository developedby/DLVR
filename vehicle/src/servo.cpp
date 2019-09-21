#include "servo.hpp"
#include <wiringPi.h>

Servo::Servo(double pin) : pwm_pin(pin)
{
    assert(pwm_pin == 1 || pwm_pin == 26 || pwm_pin == 23 || pwm_pin == 24);
    pinMode(pwm_pin, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(384);  // 50kHz (tick de 20us)
    pwmSetRange(1000);  // Pulsos de 20us a 20ms
    pwmWrite(servo, 75);  // Deixa no meio
}

// Manda o servo pra uma posição e trava ele lá
// Pra soltar o motor precisa usar Servo::release()
// pos é um valor de 0 (posição minima) a 1 (posição máxima)
void Servo::goPos(double const pos)
{
    pwmWrite(pwm_pin, (max_pos-min_pos)*pos + min_pos);
    crnt_pos = pos;
}

// Solta o servo. Na pratica, manda pwm com duty cycle 0
void Servo::release()
{
    pwmWrite(pwm_pin, 0);
}