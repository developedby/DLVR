#include "vision.hpp"
#include "movement.hpp"
#include <iostream>
#include <pigpio.h>
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
    movement.turn(angle);
    std::cout << "andando reto" <<std::endl;
    movement.goStraightMm(1, distance, 300);
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
    street_lines::StreetSection chosen_street;
    street_lines::StreetSection correction_street;
    float my_angle = 0;
    float ran_distance = 0;
    float required_distance = 120;
    bool stop = false;
    vision.getCamImg();
    found_streets = vision.findStreets();
    while(!stop) //andar 120 cm com 0 graus, virar para a rua da direita, andar mais 30
    {
        previous_street = found_streets;
        //choose the street to follow
        std::vector<int> horizontal;
        std::vector<int> vertical;
        for (unsigned int i; i < previous_streets.size(); i++)
        {
            float angle = setAngleInRange(lines[group[0]][1], (10*M_PI)/180);
            if(abs(my_angle - angle) < 40)
                horizontal.push_back(i);
            else //colocar se eh esquerda ou dirita
                vertical.push_back(i);
        }
        float bigger_street = 0;
        for (auto street : horizontal)
        {
            float street_size = previous_streets[street].end_points[3] - previous_streets[street].end_points[1];
            previous_streets[street].end_points[3] - previous_streets[street].end_points[1]; 
            if ((street_size < (required_distance - ran_distance)) && (distance > max_distance))
            {
                chosen_street =  previous_streets[street];
                bigger_street = street_size;
            }
        }
        float angle = atan2(chosen_street.end_points[3], chosen_street.end_points[2]) * (180/M_PI);
        if(angle >= 0)
            my_angle = angle;
        else if((angle + 180) < abs(angle))
            my_angle = (angle + 180);
        else
            my_angle = angle;
        ran_distance += 10;
        goAhead(movement, my_angle, 100);
        
        
        //correct the robot
        vision.getCamImg();
        found_streets = vision.findStreets();
        std::cout << "linhas encontradas" <<std::endl;
        float min_distance = 1000;
        float min_angle = 360;
        for (auto street: found_streets)
        {
            std::cout << int(street.type) << ' ' << street.line << ' ' << street.end_points << std::endl;
            float angle = setAngleInRange(street.lines[1], (10*M_PI)/180);
            float distance = street_lines::distXYPointXYSegment(cv::Point(chosen_street.end_points[2], chosen_street.end_points[3]), street.end_points);
            if((distance < min_distance) && (abs(angle - my_angle) < 20))
            {
                correction_street = street;
                min_distance = distance;
                min_angle = angle;
            }
        }
        ran_distance = ran_distance - 10 + abs(correction_street.end_points[3] - chosen_street.end_points[3]);
        my_angle -= (correction_street.line[1] - chosen_street.line[1]) * (180/M_PI)
        std::cout << "achei que andei 10 mas na verdade andei " << abs(correction_street.end_points[3] - chosen_street.end_points[3]) << " e meu angulo eh " << correction_street.line[1] - chosen_street.line[1] << " ao inves de " << chosen_street.line[1] <<std::endl;
        if(ran_distance >= required_distance)
            stop = true;
    }
    gpioTerminate();
    return 0;
}
