#include <iostream>
#include <pigpio.h>
#include "pidz.hpp"
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
float const Ts_ms = 100;


bool invert = false;
float r  = 500.0f;
bool verbose = true;
int vcnt = 0;

//Lpid Lwheel Rpid RWheel
void* params[4] = {nullptr, nullptr, nullptr, nullptr};

float vl = 0;
float vr = 0;

void tick(void* args) {
    float aux, lerr, rerr, ldc, rdc;
	PIDZ *l_pid = ((PIDZ**)args)[0];
    Wheel *l_wheel = ((Wheel**)args)[1];
    PIDZ *r_pid = ((PIDZ**)args)[2];
    Wheel *r_wheel = ((Wheel**)args)[3];
    
    aux = vl = (4*vl + l_wheel->getSpeed())/5;
    aux = (aux < 3.7f)?0.0f:aux;
    aux = (aux > 1000.0f)?(aux/2):aux;
    lerr = r - aux;
    
    aux = vr = (4*vr + r_wheel->getSpeed())/5;
    aux = (aux < 3.7f)?0.0f:aux;
    aux = (aux > 1000.0f)?(aux/2):aux;
    rerr = r - aux;
    
    ldc = l_pid->push_error(lerr);
    ldc = (ldc>1)?1:ldc;
    ldc = (ldc<0)?0:ldc;
    
    rdc = r_pid->push_error(rerr);
    rdc = (rdc>1)?1:rdc;
    rdc = (rdc<0)?0:rdc;
    if(invert){
        r_wheel->spin(1, rdc);
        l_wheel->spin(1, ldc);
    }else{
        l_wheel->spin(1, ldc);
        r_wheel->spin(1, rdc);
    }  
    invert = !invert;
    if(verbose) {
        std::cout << "    LY: "<< r - lerr <<" L_Error: " << lerr << " L_Out: " << ldc << std::endl;
        std::cout << "    RY: "<< r - rerr <<" R_Error: " << rerr << " R_Out: " << rdc << std::endl;
    }
}

void sighandler_terminate(int sig) {
    std::cout << "Terminating" << std::endl;
    gpioTerminate();
    exit(sig);
}

int main(void) {
    signal(SIGINT, sighandler_terminate);
    //float l_Kp = 1, l_Ki = 0, l_Kd = 0;
    //float r_Kp = 0.1, r_Ki = 10, r_Kd = 1;
    //float l_Kp = 1, l_Ki = 0.1, l_Kd = 0.001;
    //float r_Kp = 1, r_Ki = 0.1, r_Kd = 0.001;
    float l_Kp = 0.00004, l_Ki = 0.0018 * 0.9, l_Kd = 0.0;
    float r_Kp = 0.00005, r_Ki = 0.0020 * 0.9, r_Kd = 0.0;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    
    PIDZ l_pid(l_Kp, l_Ki, l_Kd, Ts_ms/1000.0f);
    Wheel l_wheel(l_enc_pin, l_motor_fwd_pin, l_motor_bkwd_pin, l_motor_pwm_pin);
    
    PIDZ r_pid(r_Kp, r_Ki, r_Kd, Ts_ms/1000.0f);
    Wheel r_wheel(r_enc_pin, r_motor_fwd_pin, r_motor_bkwd_pin, r_motor_pwm_pin);
    
    params[0] = &l_pid;
    params[1] = &l_wheel;
    params[2] = &r_pid;
    params[3] = &r_wheel;
    
    l_wheel.spin(1, 0.4f);
    r_wheel.spin(1, 0.3f);
    gpioSetTimerFuncEx(SAMPLE_TIMER, Ts_ms, tick, params);
    
    while(1) {
        //r = 600;
        //std::cin >> l_Kp >> l_Ki >> l_Kd;
        //l_pid.tunning(l_Kp, l_Ki, l_Kd);
        //verbose = true;

    }
	return 0;
}
