#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>
#include "geometry.hpp"
#include "constants.hpp"

float setAngleInRange(float angle, float precision)
{
    float result_angle = angle;
    if (angle > M_PI)
    {
        angle = angle - 2*M_PI;
        result_angle = angle;
    }
    if (angle > (M_PI-precision))
    {
        result_angle = M_PI - angle;
    }
    return (result_angle * 180) / M_PI;
}

void tick(void* args) {
	((Movement*)args)->tick();
}


void goAhead(std::vector<street_lines::StreetSection>& found_streets, Movement& movement, float precision)
{
    street_lines::StreetSection closed_street;
    float min_angle = 500;
    std::cout << "linha encontradas " << std::endl;
    bool found = false;
    for(auto sec : found_streets)
    {
        float angle = setAngleInRange(sec.line[1], (20*M_PI)/180);
        std::cout << sec.end_points << " angulo: " << angle <<std::endl;
        if(abs(angle) < min_angle)
        {
            found = true;
            if(angle >= 0)
                min_angle = angle;
            else if((angle + 180) < abs(angle))
                min_angle = (angle + 180);
            else
                min_angle = angle;
            closed_street = sec;
        }
    }
    if(!found)
    {
        return;
    }
    std::cout <<std::endl <<"seguindo a linha " << closed_street.end_points << "com angulo " << closed_street.line[1] <<" mas achando angulo" << min_angle <<std::endl;
    if(abs(min_angle) > precision)
    {
        movement.goStraight(0, 0);
        if(min_angle >= 0)
            movement.turn(-precision/2);
        else
            movement.turn(precision/2);
        //movement.goCurve(1, found_streets[0].direction);
    }
    else
    {
        std::cout << "andando reto" <<std::endl;
        movement.goStraight(1, 300);
    }
}

int main ()
{
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Vision vision = Vision();
    Movement movement = Movement();
    gpioSetTimerFuncEx(9, constants::pid_T_ms, tick, (void*)&movement);
    while(1)
    {
        //movement.goStraight(0, 0);
        vision.getCamImg();
        auto found_streets = vision.findStreets();
        std::cout << "antes" <<std::endl;
        for (auto sec: found_streets)
        {
            std::cout << int(sec.type) << ' ' << sec.line << ' ' << sec.end_points << std::endl;
        }
        if(!found_streets.empty())
        {
            goAhead(found_streets, movement, 10);
        }
        else
        {
            movement.goStraight(0, 0);
        }
    }
    gpioTerminate();
    return 0;
}
