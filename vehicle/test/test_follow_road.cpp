#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>
#include <math.h>
#include "geometry.hpp"
#include "constants.hpp"
#include "reduce_lines.hpp"
#include <vector>

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


void goAhead(Movement& movement, float angle, float distance)
{
    std:: cout << "girando com angulo " << angle <<std::endl;
    if(std::abs(angle) > 5)
    {
        movement.turn(angle/3);
    }
    gpioDelay(500000);
    std::cout << "andando reto" <<std::endl;
    movement.goStraightMm(1, distance, 500);
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
    std::vector<street_lines::StreetSection> previous_streets;
    std::vector<street_lines::StreetSection> found_streets;
    float my_angle = 0;
    float ran_distance = 0;
    float required_distance = 110;
    bool stop = false;
    float previous_angle = 0;
    vision.getCamImg();
    found_streets = vision.findStreets();
    while(!stop) //andar 120 cm com 0 graus, virar para a rua da direita, andar mais 30
    {
        street_lines::StreetSection chosen_street;
        street_lines::StreetSection correction_street;
        previous_streets = found_streets;
        //choose the street to follow
        std::vector<int> horizontal;
        std::vector<int> vertical;
        std::cout << "escolhendo a rua para seguir"<<std::endl;
        for (unsigned int i=0; i < previous_streets.size(); i++)
        {
            float angle = setAngleInRange(previous_streets[i].line[1], (20*M_PI)/180);
            //std::cout << previous_streets[i].line << ' ' << previous_streets[i].end_points << std::endl << " angulo eh " <<angle <<std::endl;
            if(std::abs(my_angle - angle) < 40)
            {
                horizontal.push_back(i);
                //std::cout << "essa rua eh horizontal" << std::endl;
            }
            else //colocar se eh esquerda ou dirita
                vertical.push_back(i);
        }
        float bigger_street_size = 0;
        for (auto street : horizontal)
        {
            float street_size = std::abs(previous_streets[street].end_points[3] - previous_streets[street].end_points[1]);
            street_size = std::isinf(street_size) ? 0 : street_size;
            if ((street_size < (required_distance - ran_distance)) && (street_size > bigger_street_size))
            {
                chosen_street =  previous_streets[street];
                bigger_street_size = street_size;
            }
        }
        if(chosen_street.line[0] != 0)
        {
            std::cout << "Escolhi a rua " <<int(chosen_street.type) << ' ' << chosen_street.line << ' ' << chosen_street.end_points << std::endl;
            float angle = atan2(chosen_street.end_points[3], chosen_street.end_points[2]) * (180/M_PI) - 90;
            previous_angle = my_angle;
            if(angle >= 0)
                my_angle = angle;
            else if((angle + 180) < std::abs(angle))
                my_angle = (angle + 180);
            else
                my_angle = angle;
            ran_distance += 10;
            goAhead(movement, my_angle, 70);
        }
        else //nao encontrou nenhuma rua horizontal
        {
            if(my_angle > 0)
                movement.turn(-2);
            else
                movement.turn(2);
        }
        
        
        //correct the robot
        vision.getCamImg();
        gpioDelay(200000);
        found_streets = vision.findStreets();
        /*std::cout << "linhas encontradas" <<std::endl;
        float min_distance = 1000;
        //float min_angle = 360;
        for (auto street: found_streets)
        {
            std::cout << int(street.type) << ' ' << street.line << ' ' << street.end_points << std::endl;
            if(chosen_street.line[0] == 0)
            {
                std::cout << "A linha eh zero" <<std::endl;
                break;
            }
            if((street.line[0] != 0) && (!std::isinf(street.end_points[3])) && (!std::isinf(street.end_points[2])))
            {
                std::cout << int(street.type) << ' ' << street.line << ' ' << street.end_points << std::endl;
                float angle = setAngleInRange(street.line[1], (20*M_PI)/180);
                float distance = street_lines::distXYPointXYSegment(cv::Point(chosen_street.end_points[2], chosen_street.end_points[3]), street.end_points);
                if((distance < min_distance) && (std::abs(angle - my_angle) < 45))
                {
                    correction_street = street;
                    min_distance = distance;
                    min_angle = angle;
                }
            }
        }
        if((chosen_street.line[0] != 0) && (correction_street.line[0] != 0))
        {
            std::cout << "Escolhi para correcao a rua " <<int(correction_street.type) << ' ' << correction_street.line << ' ' << correction_street.end_points << std::endl;
            float real_distance = (correction_street.end_points[3] - chosen_street.end_points[3]) * 100;
            ran_distance = ran_distance - 10 + std::abs(real_distance);
            float angle2 = atan2(correction_street.end_points[3], correction_street.end_points[2]) * (180/M_PI) - 90;
            std::cout << "achei que andei 10 mas na verdade andei " << real_distance << " com " << std::abs(real_distance) << " e meu angulo eh " << my_angle - previous_angle + angle2 << " ao inves de " << my_angle <<std::endl;
            my_angle = my_angle - previous_angle + angle2;
        }*/
        std::cout << "ate agora andei " << ran_distance <<std::endl;
        if(ran_distance >= required_distance)
            stop = true;
    }
    gpioTerminate();
    return 0;
}
