#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>


void goAhead(std::vector<street_lines::StreetSection> found_streets, Movement movement)
{
    street_lines::StreetSection closed_street;
    int min_dist = 999999;
    for(int i = 0; i<found_streets.size(); i++)
    {
        if(distXYPointXYSegment(cv::Point(0, 0), found_streets[i].end_points) < min_dist)
        {
            min_dist = distXYPointXYSegment(cv::Point(0, 0), found_streets[i].end_points);
            closed_street = found_streets[i];
        }
    }
    movement.goStraight(1, 0.2);
    if(closed_street.line[1] > 10)
    {
        movement.goStraight(0, 0);
        movement.turn(found_streets[0].line[1]);
        //movement.goCurve(1, found_streets[0].direction);
    }
    else if(closed_street.line[1] < -10)
    {
        movement.goStraight(0, 0);
        movement.turn(-found_streets[0].line[1]);
        //movement.goCurve(1, -found_streets[0].direction);
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
    std::vector<street_lines::StreetSection> found_streets;
    while(1)
    {
        vision.getCamImg();
        found_streets = vision.findStreets();
        if(!found_streets.empty())
        {
            goAhead(found_streets)
        }
        else
        {
            movement.goStraight(0, 0);
        }
    }
    return 0;
}
