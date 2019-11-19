#include "movement.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <pigpio.h>
#include "vision.hpp"
#include "geometry.hpp"
#include "constants.hpp"

float setAngleInRange(float angle, float precision)
{
    float result_angle = angle;
    if (angle > M_PI)
    {
        //std::cout << "angle " << angle <<" eh maior que " << M_PI <<" entao agora ele eh " << angle - 2*M_PI <<std::endl;
        angle = angle - 2*M_PI;
        result_angle = angle;
    }
    if (std::abs(angle) > (M_PI-precision))
    {
        //std::cout << "angle " << angle << " eh maior que a pricisao entao agora ele eh " << M_PI - std::abs(angle) <<std::endl;
        result_angle = M_PI - std::abs(angle);
    }
    return (result_angle * 180) / M_PI;
}


void goAhead(Movement& movement, float angle, float distance)
{
    std:: cout << "girando com angulo " << angle <<std::endl;
    if(std::abs(angle) > 5)
    {
        //movement.turn(angle/3);
    }
    gpioDelay(500000);
    std::cout << "andando reto" <<std::endl;
    //movement.goStraightMm(1, distance, 200);
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
    std::vector<street_finder::StreetSection> previous_streets;
    std::vector<street_finder::StreetSection> found_streets;
    std::vector<street_finder::StreetSection> previous_tapes;
    std::vector<street_finder::StreetSection> found_tapes;
    float my_angle = 0;
    float ran_distance = 0;
    float required_distance = 20;//110;
    bool stop = false;
    float previous_angle = 0;
    /*for(int i = 0; i<4; i++)
    {
        movement.goStraightMm(1, 500, 200);
        gpioDelay(2000000);
        std::cout << "parei" <<std::endl;
        movement.turn(90);
        gpioDelay(1000000);
    }*/
        
    vision.getDownwardCamImg();
    std::tie(found_tapes, found_streets) = vision.findStreets();
    while(!stop) //andar 120 cm com 0 graus, virar para a rua da direita, andar mais 30
    {
        street_finder::StreetSection chosen_street;
        street_finder::StreetSection correction_street;
        street_finder::StreetSection close_tape;
        street_finder::StreetSection correction_tape;
        cv::Point2f chosen_point;
        cv::Point2f correction_point;
        previous_streets = found_streets;
        previous_tapes = found_tapes;
        //choose the street to follow
        std::vector<int> horizontal_street;
        std::vector<int> vertical_street;
        std::vector<int> horizontal_tape;
        std::cout << "escolhendo a rua para seguir"<<std::endl;
        
        //classify streets by angle
        for (unsigned int i=0; i < previous_streets.size(); i++)
        {
            float angle = setAngleInRange(previous_streets[i].line[1], (20*M_PI)/180);
            //std::cout << previous_streets[i].line << ' ' << previous_streets[i].seg << std::endl << " angulo eh " <<angle <<std::endl;
            if(std::abs(my_angle - angle) < 40)
            {
                horizontal_street.push_back(i);
                //std::cout << "essa rua eh horizontal" << std::endl;
            }
            else //colocar se eh esquerda ou dirita
                vertical_street.push_back(i);
        }
        
        //get horizontal tapes
        for (unsigned int i=0; i < previous_tapes.size(); i++)
        {
            float angle = setAngleInRange(previous_tapes[i].line[1], (20*M_PI)/180);
            //std::cout << previous_tapes[i].line << ' ' << previous_tapes[i].seg <<" " << " angulo eh " <<angle <<std::endl;
            if(std::abs(my_angle - angle) < 40)
            {
                horizontal_tape.push_back(i);
                //std::cout << "essa fita eh horizontal" << std::endl;
            }
        }
        
        //choose street to follow
        float further_point = 0;
        for (auto street : horizontal_street)
        {
            float fair = std::max(previous_streets[street].seg[3], previous_streets[street].seg[1]);
            if (fair > further_point)
            {
                chosen_street =  previous_streets[street];
                further_point = fair;
            }
        }
        
        //choose close tape to compare later and get the real run distance
        float minimal_distance = 200;
        for (auto tape : horizontal_tape)
        {
            float distance = std::min(previous_tapes[tape].seg[1], previous_tapes[tape].seg[3]);
            if(distance < minimal_distance)
            {
                close_tape = previous_tapes[tape];
                minimal_distance = distance;
            }
        }
        std::cout <<"fita mais proxima: " << int(close_tape.color) << " "<<close_tape.seg <<std::endl; 
        
        //follow street
        if(chosen_street.line[0] != 0)
        {
            if(chosen_street.seg[3] > chosen_street.seg[1])
                chosen_point = cv::Point2f(chosen_street.seg[2], chosen_street.seg[3]);
            else
                chosen_point = cv::Point2f(chosen_street.seg[0], chosen_street.seg[1]);
            float angle = atan2(chosen_point.y, chosen_point.x) * (180/M_PI) - 90;
            std::cout << "Escolhi a rua " << int(chosen_street.color) << ' ' << chosen_street.line << ' ' << chosen_street.seg << " angulo: " << angle << std::endl ;
            
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
        vision.getDownwardCamImg();
        std::tie(found_tapes, found_streets) = vision.findStreets();
        std::cout << "linhas encontradas" <<std::endl;
        
        //choose street to correcty angle
        float min_distance = 1000;
        //float min_angle = 360;
        for (auto street: found_streets)
        {
            std::cout << int(street.color) << ' ' << street.line << ' ' << street.seg << std::endl;
            if(chosen_street.line[0] == 0)
            {
                std::cout << "A linha eh zero" <<std::endl;
                break;
            }
            if((street.line[0] != 0) && (!std::isinf(street.seg[3])) && (!std::isinf(street.seg[2])))
            {
                float angle = setAngleInRange(street.line[1], (20*M_PI)/180);
                float distance = geometry::distPointSegment(chosen_point, street.seg);
                if((distance < min_distance) && (std::abs(angle - previous_angle) < 40))
                {
                    //std::cout << "entrei aqui"<<std::endl;
                    correction_street = street;
                    min_distance = distance;
                    //min_angle = angle;
                }
            }
        }
        
        //try find continuation of the close tape
        minimal_distance = 200;
        std::cout << "fitas encontradas" <<std::endl;
        for(auto tape: found_tapes)
        {
            std::cout << int(tape.color) << ' ' << tape.seg << std::endl;
            if (tape.color == close_tape.color)
            {
                float distance = std::min(tape.seg[1], tape.seg[3]);
                if(distance < minimal_distance)
                {
                    correction_tape = tape;
                    minimal_distance = distance;
                } 
            }
        }
        std::cout <<"fita para correcao: " << int(correction_tape.color) << " "<<correction_tape.seg <<std::endl; 
        
        //correct the vehicle angle
        if((chosen_street.line[0] != 0) && (correction_street.line[0] != 0))
        {
            if(correction_street.seg[3] > correction_street.seg[1])
                correction_point = cv::Point2f(correction_street.seg[2], correction_street.seg[3]);
            else
                correction_point = cv::Point2f(correction_street.seg[0], correction_street.seg[1]);
            float angle2 = atan2(correction_point.y, correction_point.x) * (180/M_PI) - 90;
            std::cout << "Escolhi para correcao a rua " <<int(correction_street.color) << ' ' << correction_street.line << ' ' << correction_street.seg  << " angulo: " << angle2<< std::endl;
            std::cout << "meu angulo eh " << my_angle - previous_angle - angle2 << " ao inves de " << my_angle <<std::endl;
            my_angle = my_angle - previous_angle - angle2;
            previous_angle = my_angle;
            //my_angle = 0;
        }
        //correct the vehicle run distance
        if((close_tape.line[0] != 0) && (correction_tape.line[0] != 0))
        {
            float real_distance = (std::max(correction_tape.seg[1], correction_tape.seg[3]) - std::max(close_tape.seg[1], close_tape.seg[3])) * 100;
            ran_distance = ran_distance - 10 + std::abs(real_distance);
            std::cout << "achei que andei 10 mas na verdade andei " << real_distance << std::endl;
        }
        std::cout << "ate agora andei " << ran_distance <<std::endl;
        if(ran_distance >= required_distance)
            stop = true;
    }
    /*bool found = false;
    while(!found)
    {
        vision.getTopCamImg();
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> positions;
        std::tie(ids, positions) = vision.findARMarkers();
        for(unsigned int i=0; i < ids.size(); i++)
        {
            if(ids[i] == 1)
            {
                found = true;
                break;
            }
        }
        movement.turn(5);
        gpioDelay(500000);
        if(found)
        {
            movement.goStraightMm(1, 70, 500);
            gpioDelay(500000);
            movement.goStraightMm(1, 70, 500);
            gpioDelay(500000);
        }
    }*/
    gpioTerminate();
    return 0;
}
