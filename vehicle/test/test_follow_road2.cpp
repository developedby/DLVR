#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>
#include "geometry.hpp"

float setAngleInRange(float angle, float precision)
{
    float result_angle = angle;
    if ((angle > (M_PI-precision)) & (angle < (M_PI-precision)))
    {
        result_angle = M_PI - angle;
    }
    else if (angle > M_PI)
    {
        result_angle = angle - 2*M_PI;
    }
    return (result_angle * 180) / M_PI;
}


void goAhead(std::vector<street_lines::StreetSection> found_streets, Movement& movement, float precision)
{
    street_lines::StreetSection closed_street;
    int min_dist = 999999;
    for(unsigned int i = 0; i<found_streets.size(); i++)
    {
        if(street_lines::distXYPointXYSegment(cv::Point(0, 0), found_streets[i].end_points) < min_dist)
        {
            min_dist = street_lines::distXYPointXYSegment(cv::Point(0, 0), found_streets[i].end_points);
            closed_street = found_streets[i];
        }
    }
    float adjusted_angle = setAngleInRange(closed_street.line[1], (precision*M_PI)/180);
    //std::cout << "seguindo a linha " << closed_street.end_points << "com angulo " << closed_street.line[1] <<" mas achando angulo" << adjusted_angle <<std::endl;
    movement.goStraight(1, 0.2);
    if(abs(adjusted_angle) > precision)
    {
        movement.goStraight(0, 0);
        movement.turn(adjusted_angle);
        //movement.goCurve(1, found_streets[0].direction);
    }
}

int main ()
{
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    //Vision vision = Vision();
    Movement movement;// = Movement();
    std::vector<street_lines::StreetSection> found_streets;
    while(1)
    {
        movement.turn(90);
		gpioDelay(1000000);
		movement.turn(-90);
		gpioDelay(1000000);
        //movement.turn(90);
        movement.goStraight(0, 0);
        std::cout << "parando" << std::endl;
        break;
        /*vision.getCamImg();
        found_streets = vision.findStreets();
        if(!found_streets.empty())
        {
            goAhead(found_streets, movement, 10);
        }
        else
        {
            movement.goStraight(0, 0);
        }*/
    }
    gpioTerminate();
    return 0;
}
