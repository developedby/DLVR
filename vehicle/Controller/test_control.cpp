#include "movement.hpp"
#include <pigpio.h>

float const wheel_distance = 128.9;
int const r_enc_pin = 3;
int const l_motor_fwd_pin = 21;
int const l_motor_bkwd_pin = 20;
int const r_motor_pwm_pin = 18;

int const l_enc_pin = 26;
int const r_motor_fwd_pin = 17;
int const r_motor_bkwd_pin = 7;
int const l_motor_pwm_pin = 12;

int const SAMPLE_TIMER = 9;
float const Ts = 100;

void tick(void* movement) {
	((Movement*)movement)->tick();
}

int main(void) {
	if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Movement movement(128.9f,
		      1.0, 0, 0,	//PID Left
		      0.6, 0, 0,	//PID Right
		      Ts/1000,
		      l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin,
		      r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin
		      );

	movement.goStraight(1, 500);
	gpioSetTimerFuncEx(SAMPLE_TIMER, Ts, tick, &movement);
	while(1);
	return 0;
}
