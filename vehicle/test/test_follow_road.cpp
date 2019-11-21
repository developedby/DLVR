#include "movement.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <pigpio.h>
#include "vision.hpp"
#include "geometry.hpp"
#include "constants.hpp"

constexpr float dist_per_step_m = 0.1;

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
    if(std::abs(angle) > 1)
    {
        movement.turn(angle);
    }
    //gpioDelay(500000);
    std::cout << "andando reto" <<std::endl;
    movement.goStraightMm(1, distance, 200);
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
    float required_distance = 100;//110;
    bool stop = false;
    float previous_angle = 0;
        
    vision.getDownwardCamImg();
    auto [found_tapes, found_streets] = vision.findStreets();
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
        std::vector<street_finder::StreetSection> prev_tape_sorted_x;
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
        
        //get horizontal tapes and sorted by x
        for (auto tape: previous_tapes)
        {
            float angle = setAngleInRange(tape.line[1], (20*M_PI)/180);
            //std::cout << previous_tapes[i].line << ' ' << previous_tapes[i].seg <<" " << " angulo eh " <<angle <<std::endl;
            if(std::abs(my_angle - angle) < 40)
            {
                if(tape.seg[0] < 0)
                    prev_tape_sorted_x[0].emplace_back(tape);
                else
                    prev_tape_sorted_x[1].emplace_back(tape);
            }
        }
        
        // Choose the next point to aim for
        // Find the average line of all the lines considered to be parallel
        // Separate the segment points
        std::vector<Vec2f> horizontal_street_pts;
        for (const auto street: horizontal_street)
        {
            horizontal_street_pts.emplace_back(previous_street[street].seg[0],
                                               previous_street[street].seg[1]);
            horizontal_street_pts.emplace_back(previous_street[street].seg[2],
                                               previous_street[street].seg[3]);
        }
        // Sort the points
        std::sort(horizontal_street_pts.begin(), horizontal_street_pts.end(),
                  [](auto pt1, auto pt2){return pt1[1] < pt2[1];});
        // Separate the points in two groups, calculating the average of each group
        // The segment is the two average points
        cv::Vec4f avg_seg(0, 0, 0, 0);
        int num_pts = horizontal_street_pts.size();
        int pt_counter = 0;
        for (const auto& pt : horizontal_street_pts)
        {
            if (pt_counter < num_pts/2)
            {
                avg_seg[0] += pt[0];
                avg_seg[1] += pt[1];
            }
            else
            {
                avg_seg[2] += pt[0];
                avg_seg[3] += pt[1];
            }
            pt_counter++;
        }
        if (num_pts)
        {
            avg_seg[0] /= num_pts/2;
            avg_seg[1] /= num_pts/2;
            avg_seg[2] /= num_pts/2;
            avg_seg[3] /= num_pts/2;
        }
        const Vec2f avg_line = geometry::segmentToLine(avg_seg);
        // Project that line and find the point on it that is the distance the vehicle wants to move
        // The line is defined as 'pt0 + u*delta'
        const Vec2f pt0(avg_line[0]*cos(avg_line[1]), avg_line[0]*sin(avg_line[1]));
        const Vec2f delta(-sin(avg_line[1]), cos(avg_line[1]));
        const float u = norm(avg_line[0], dist_per_step_m);
        const Vec2f next_pt = (delta[1] > 0) ? (pt0 + u*delta) : (pt0 - u*delta);
        
        float angle = atan2(next_point[1], next_point[0]) * (180/M_PI) - 90;
        std::cout << "Escolhi a rua " << avg_line << ' ' << avg_seg << " angulo: " << angle << std::endl;
            
        previous_angle = my_angle;
        if(angle >= 0)
            my_angle = angle;
        else if((angle + 180) < std::abs(angle))
            my_angle = (angle + 180);
        else
            my_angle = angle;
        
        //choose close tape to compare later and get the real run distance
        //tem que tentar ordenar pq n esta funcionando
        float minimal_distance = 200;
        std::sort(prev_tape_sorted_x[0].begin(), prev_tape_sorted_x[0].end(),
                  [](auto sec1, auto sec2)
                  {
                      const float y1 = (sec1.seg[3] > sec1.seg[1]) ? sec1.seg[1] : sec1.seg[3];
                      const float y2 = (sec2.seg[3] > sec2.seg[1]) ? sec2.seg[1] : sec2.seg[3];
                      return y1 < y2;
                  });
          
        std::sort(prev_tape_sorted_x[1].begin(), prev_tape_sorted_x[1].end(),
                  [](auto sec1, auto sec2)
                  {
                      const float y1 = (sec1.seg[3] > sec1.seg[1]) ? sec1.seg[1] : sec1.seg[3];
                      const float y2 = (sec2.seg[3] > sec2.seg[1]) ? sec2.seg[1] : sec2.seg[3];
                      return y1 < y2;
                  });
        for (auto tape : previous_tape)
        {
            
        }
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
        if(num_pts)
        {
            ran_distance += 10;
            goAhead(movement, my_angle, 70);
            movement.turn(-(my_angle - avg_line[1]));
        }
        else //nao encontrou nenhuma rua horizontal
        {
            std::cout << "girando pra ver se encontra uma linha" <<std::endl;
            if(my_angle > 0)
                movement.turn(-15);
            else
                movement.turn(15);
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
            //ran_distance = ran_distance - 10 + std::abs(real_distance);
            std::cout << "achei que andei 10 mas na verdade andei " << real_distance << std::endl;
        }
        std::cout << "ate agora andei " << ran_distance <<std::endl;
        if(ran_distance >= required_distance)
            stop = true;
    }

    gpioTerminate();
    return 0;
}
