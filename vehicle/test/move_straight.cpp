#include "wheel.hpp"
#include <pigpio.h>
#include <iostream>

//#define SPEED_TO_UPDATE_TIME 3
#define REQUIRED_SPEED 200
#define D_SPEED 0.001

Wheel *lw;
Wheel *rw;
float lm_speed = 0.5;
float rm_speed = 0.5;
int l_previous_read = 0;
int r_previous_read = 0;
int cont = 0;

void move(void)
{
	//int l_num_ticks = lw->encoder.ticks - l_previous_read;
	//int r_num_ticks = rw->encoder.ticks - r_previous_read;
	//l_previous_read = lw->encoder.ticks;
	//r_previous_read = rw->encoder.ticks;
	//std::cout << "cont l: " << l_num_ticks << std::endl;
	//std::cout << "cont r: " << r_num_ticks << std::endl;
	cont++;
	if(cont<20)
	{
		std::cout << "l: " << lw->getSpeed() << std::endl;
		std::cout << "r: " << rw->getSpeed() << std::endl;
		int l_error = lw->getSpeed() - REQUIRED_SPEED;
		int r_error = rw->getSpeed() - REQUIRED_SPEED;
		lm_speed -= l_error*D_SPEED;
		rm_speed -= r_error*D_SPEED;
		if (l_error > r_error) //roda esquerda esta mais rapida
		{
			lm_speed -= 0.01;
		}
		else if(r_error > l_error)
		{
			rm_speed -= 0.01;
		}
		if(lm_speed < 0)
		{
			lm_speed = 0;
		}
		else if(lm_speed > 1)
		{
			lm_speed = 1;
		}
		if(rm_speed < 0)
		{
			rm_speed = 0;
		}
		else if(rm_speed > 1)
		{
			rm_speed = 1;
		}
	}
	else if(cont > 30)
	{
		cont = 0;
	}
	/*else
	{
		lm_speed += D_SPEED;
		if(lm_speed > 1)
		{
			lm_speed = 1;
		}
	}
	if(rw->getSpeed()>REQUIRED_SPEED)
	{
		rm_speed -= D_SPEED;
		if(rm_speed < 0)
		{
			rm_speed = 0;
		}
	}
	else
	{
		rm_speed += D_SPEED;
		if(rm_speed > 1)
		{
			rm_speed = 1;
		}
	}*/
}

int main()
{
	if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    lw = new Wheel(0);
    rw = new Wheel(1);
    lw->spin(0, 0);
    rw->spin(0, 0);
    gpioDelay(1000000);
	gpioSetTimerFunc(0, 250, move);
	while(true)
	{
		//std::cout << lm_speed << " " << rm_speed <<std::endl;
		if(cont > 20)
		{
			lw->spin(0, 0);
			rw->spin(0, 0);
		}
		else
		{
			lw->spin(1, lm_speed);
			rw->spin(1, rm_speed);
		}
			
	}
	gpioTerminate();
	return 0;
}
