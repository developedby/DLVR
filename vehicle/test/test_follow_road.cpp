#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>

int main ()
{
    Vision vision = Vision();
    Movement movement = Movement();
    std::vector<street_lines::StreetSection> found_streets;
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    while(1)
    {
        found_streets = vision.findStreets();
        if(!found_streets.empty() && (found_streets[0].type == street_lines::SECTION_STREET))
        {
            movement.goStraight(1, 0.2);
            if(found_streets[0].direction > 10)
            {
                movement.goStraight(0, 0);
                movement.turn(found_streets[0].direction);
                //movement.goCurve(1, found_streets[0].direction);
            }
            else if(found_streets[0].direction < -10)
            {
                movement.goStraight(0, 0);
                movement.turn(-found_streets[0].direction);
                //movement.goCurve(1, -found_streets[0].direction);
            }
        }
        else
        {
            movement.goStraight(0, 0);
        }
    }
    return 0;
}
