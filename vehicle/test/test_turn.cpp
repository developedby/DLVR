#include <iostream>
#include <pigpio.h>
#include "movement.hpp"
#include <csignal>
#include "constants.hpp"

void tick(void* args) {
	((Movement*)args)->tick();
}

void sighandler_terminate(int sig) {
    std::cout << "Terminating" << std::endl;
    gpioTerminate();
    exit(sig);
}

int main(void) {
    std::cout << "SIGFUNC: " << signal(SIGINT, sighandler_terminate) << std::endl;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Movement mov;
    gpioSetTimerFuncEx(9, constants::pid_T_ms, tick, (void*)&mov);
    float dir = 1.0;
    float angle = 0.0f;
    while(1) {
		mov.turn(90);
		gpioDelay(10000000);
		mov.turn(-90);
		gpioDelay(10000000);
		mov.turn(180);
		gpioDelay(10000000);
		mov.turn(-180);
		gpioDelay(10000000);
		mov.turn(180);
		gpioDelay(10000000);
		mov.turn(-180);
		while(mov.isTurning());
		gpioDelay(100);
		break;
	}
    std::cout << "Terminating" << std::endl;
    mov.goStraight(0,0);
    gpioTerminate();
    return 0;
}
