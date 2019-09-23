#include "servo.hpp"
#include <cassert>
#include <pthread.h>
#include <wiringPi.h>

Servo::Servo(int const pin) : pwm_pin(pin)
{
    assert(pwm_pin == 1 || pwm_pin == 26 || pwm_pin == 23 || pwm_pin == 24);
    pinMode(pwm_pin, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(384);  // 50kHz (tick de 20us)
    pwmSetRange(1000);  // Pulsos de 20us a 20ms
    delay(500);
}

// Manda o servo pra uma posição e trava ele lá
// pos é um valor de 0 (posição minima) a 1 (posição máxima)
void Servo::goPos(double const pos)
{
    pwmWrite(pwm_pin, (max_pos-min_pos)*pos + min_pos);
    crnt_pos = pos;
    pthread_t release_thread;
    pthread_create(&release_thread, nullptr, &scheduleRelease, (void*)this);
}

// Solta o servo. Na pratica, manda pwm com duty cycle 0
void Servo::release()
{
    pwmWrite(pwm_pin, 0);
}

void * scheduleRelease(void * servo_)
{
    delay(400);
    Servo* servo = static_cast<Servo*>(servo_);
    servo->release();
    return nullptr;
}
