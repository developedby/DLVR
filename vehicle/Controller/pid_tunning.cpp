#include <iostream>
#include <pigpio.h>
#include "pid.hpp"
#include "wheel.hpp"
#include <csignal>

float const wheel_distance = 128.9;
int const r_enc_pin = 3;
int const l_motor_fwd_pin = 7; 
int const l_motor_bkwd_pin = 17;
int const r_motor_pwm_pin = 18;

int const l_enc_pin = 26;
int const r_motor_fwd_pin = 21;
int const r_motor_bkwd_pin = 20;
int const l_motor_pwm_pin = 12;

int const SAMPLE_TIMER = 9;
float const Ts_ms = 500;


bool invert = false;
float r  = 1000.0f;
bool verbose = false;
int vcnt = 0;

//Lpid Lwheel Rpid RWheel
void* params[4] = {nullptr, nullptr, nullptr, nullptr};


void tick(void* args) {
    float aux, lerr, rerr, ldc, rdc;
	PID *l_pid = ((PID**)args)[0];
    Wheel *l_wheel = ((Wheel**)args)[1];
    PID *r_pid = ((PID**)args)[2];
    Wheel *r_wheel = ((Wheel**)args)[3];
    
    aux = l_wheel->getSpeed();
    aux = (aux < 3.7f)?0.0f:aux;
    lerr = r - aux;
    
    aux = r_wheel->getSpeed();
    aux = (aux < 3.7f)?0.0f:aux;
    rerr = r - aux;
    
    ldc = l_pid->push_error(lerr);
    rdc = r_pid->push_error(rerr);
    if(invert){
        r_wheel->spin(1, rdc);
        l_wheel->spin(1, ldc);
    }else{
        l_wheel->spin(1, ldc);
        r_wheel->spin(1, rdc);
    }  
    invert = !invert;
    if(verbose) {
        std::cout << "    Y: "<< r - lerr <<" L_Error: " << lerr << " L_Out: " << ldc;
        std::cout << "    Y: "<< r - rerr <<" R_Error: " << rerr << " R_Out: " << rdc << std::endl;
    }
}

void sighandler_terminate(int sig) {
    std::cout << "Terminating" << std::endl;
    gpioTerminate();
    exit(sig);
}

int main(void) {
    signal(SIGINT, sighandler_terminate);
    float l_Kp = 0.1, l_Ki = 10, l_Kd = 1;
    float r_Kp = 0.1, r_Ki = 10, r_Kd = 1;
    //float l_Kp = 0.0012, l_Ki = 0.02222, l_Kd = 0.0000162;
    //float r_Kp = 0.0012, r_Ki = 0.02222, r_Kd = 0.0000162;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    
    PID l_pid(l_Kp, l_Ki, l_Kd, Ts_ms/1000.0f, 0.0f, 1.0f);
    Wheel l_wheel(l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin);
    
    PID r_pid(r_Kp, r_Ki, r_Kd, Ts_ms/1000.0f, 0.0f, 1.0f);
    Wheel r_wheel(r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin);
    
    params[0] = &l_pid;
    params[1] = &l_wheel;
    params[2] = &r_pid;
    params[3] = &r_wheel;
    
    l_wheel.spin(1, 1.0f);
    r_wheel.spin(1, 1.0f);
    gpioSetTimerFuncEx(SAMPLE_TIMER, Ts_ms, tick, params);
    
    while(1) {
        /*      
        std::cout << std::endl << "Left: ";
        std::cin >> l_Kp >> l_Ki >> l_Kd;
        l_pid.tunning(l_Kp, l_Ki, l_Kd);
        
        std::cout << "Right: ";
        std::cin >> r_Kp >> r_Ki >> r_Kd;
        r_pid.tunning(r_Kp, r_Ki, r_Kd);
        std::cout << std::endl;
        */
    }
	return 0;
}
