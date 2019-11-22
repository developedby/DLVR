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

void goAhead(Movement& movement, float angle, float dist_mm)
{
    std:: cout << "girando com angulo " << angle <<std::endl;
    if(std::abs(angle) > 1)
    {
        movement.turn(angle);
    }
    //gpioDelay(500000);
    std::cout << "andando reto" <<std::endl;
    movement.goStraightMm(1, dist_mm, 200);
}

int main()
{
    if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar a GPIO!" << std::endl;
        exit(PI_INIT_FAILED);
    }
    Vision vision = Vision();
    Movement movement = Movement();
    float total_ran_dist_mm = 0;
    const float required_distance = 1000;
    bool stop = false;    
    vision.getDownwardCamImg();
    auto [found_tapes, found_streets] = vision.findStreets();
    std::cout << "Ruas encontradas: " << std::endl;
    std::for_each(found_streets.begin(), found_streets.end(), [](auto sec){sec.print();});

    std::vector<streets::StreetSection> left_tapes;
    std::vector<streets::StreetSection> right_tapes;
    for (const auto& tape: found_tapes)
    {
        const float tape_angle = setAngleInRange(tape.line[1], (20*M_PI)/180);
        //std::cout << found_tapes[i].line << ' ' << found_tapes[i].seg <<" " << " angulo eh " <<angle <<std::endl;
        if(std::abs(tape_angle) < 40)
        {
            if(tape.seg[0] < 0)  // Both seg[0] and seg[2] should give the same results
                left_tapes.push_back(tape);
            else
                right_tapes.push_back(tape);
        }
    }
    // Sort the tapes by their closest point
    streets::orderCollinearSections(left_tapes, 1);
    streets::orderCollinearSections(right_tapes, 1);
    int turn_direction = 0;
    
    while(!stop) //andar 120 cm com 0 graus, virar para a rua da direita, andar mais 30
    {
        /*
         * Choose the next point
         * Choose the tape to calculate the moved distance
         * Move to next point
         * Take a new picture
         * Calculate how much was moved
         * Correct the movement
         */
        
        /* Choose the next point to aim for */
        // Separate streets that are parallel from streets that are perpendicular to the vehicle
        std::vector<streets::StreetSection> parallel_streets;
        std::vector<streets::StreetSection> perpendicular_streets;
        std::cout << "escolhendo o ponto para seguir" << std::endl;
        for (const auto& street: found_streets)
        {
            const float street_angle = setAngleInRange(street.line[1], 20*(M_PI/180));
            //std::cout << found_streets[i].line << ' ' << found_streets[i].seg << std::endl << " angulo eh " <<angle <<std::endl;
            if(std::abs(street_angle) < 30)
                parallel_streets.push_back(street);
            else
                perpendicular_streets.push_back(street);
        }

        // Find the average line of all the lines considered to be parallel
        // Separate the segment points
        std::vector<cv::Vec2f> parallel_street_pts;
        for (const auto& street: parallel_streets)
        {
            parallel_street_pts.emplace_back(street.seg[0], street.seg[1]);
            parallel_street_pts.emplace_back(street.seg[2], street.seg[3]);
        }
        // Sort the points
        std::sort(parallel_street_pts.begin(), parallel_street_pts.end(),
                  [](auto pt1, auto pt2){return pt1[1] < pt2[1];});
        // Separate the points in two groups, calculating the average of each group
        // The segment is the two average points
        cv::Vec4f avg_seg(0, 0, 0, 0);
        const int num_pts = parallel_street_pts.size();
        int pt_counter = 0;
        for (const auto& pt : parallel_street_pts)
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
        
        // Extend the average segment to a line
        const cv::Vec2f avg_line = geometry::segmentToLine(avg_seg);
        /*// Find the point on the line that is the distance the vehicle wants to move
        // The line is defined as 'pt0 + u*delta'
        const cv::Vec2f pt0(avg_line[0]*cos(avg_line[1]), avg_line[0]*sin(avg_line[1]));
        const cv::Vec2f delta(-sin(avg_line[1]), cos(avg_line[1]));
        //const float u = geometry::norm(avg_line[0], dist_per_step_m);
        std::cout <<"valores para u " << avg_line[0] << " " << dist_per_step_m <<std::endl;
        const float u = sqrt(geometry::square(dist_per_step_m) - geometry::square(avg_line[0]));
        const cv::Vec2f next_pt = (delta[1] > 0) ? (pt0 + u*delta) : (pt0 - u*delta);*/
        // Calculate the angle to the next point, where 0 means in front
        const cv::Vec2f next_pt = cv::Vec2f(avg_seg[0], avg_seg[1]);
        const float angle = atan2(next_pt[1], next_pt[0])*(180/M_PI) - 90;
        std::cout << "Proximo ponto: " << next_pt << " Angulo: " << angle << std::endl;
        //std::cout << "Proxima rua: " << avg_line << std::endl;
               
        /* Select the next tape to use to calculate the moved distance */
        // Choose the second closest tape, if available
        streets::StreetSection left_reference_tape = streets::StreetSection();
        streets::StreetSection right_reference_tape = streets::StreetSection();
        if (left_tapes.size() >= 2)
            left_reference_tape = left_tapes[1];
        else if (left_tapes.size() == 1)
            left_reference_tape = left_tapes[0];
        else
            left_reference_tape.color = streets::Color::none;
        if (right_tapes.size() >= 2)
            right_reference_tape = right_tapes[1];
        else if (right_tapes.size() == 1)
            right_reference_tape = right_tapes[0];
        else
            right_reference_tape.color = streets::Color::none;
        std::cout << "Escolheu para calcular movimento feito as fitas:" << std::endl;
        left_reference_tape.print();
        right_reference_tape.print();
        
        /* Go to the chosen point, adjusting the angle to be parallel with the street in the end */
        float dist_to_move_mm;
        if(num_pts)
        {
            dist_to_move_mm = consts::step_size_mm;
            std::cout << "Vai andar " << dist_to_move_mm << " mm" << std::endl;
            if (std::abs(angle/2) > consts::turn_angle_threshold)
            {
                movement.turn(angle/2);
                turn_direction = 1;
                gpioDelay(300000);
            }
            movement.goStraightMm(1, dist_to_move_mm, 200);
            std::cout << "corrigindo com angulo" << -(angle - avg_line[1]) << std::endl;
            gpioDelay(300000);
            if(std::abs((angle - avg_line[1])/2) > consts::turn_angle_threshold)
            {
                movement.turn(-(angle - avg_line[1])/2);
                turn_direction = -1;
                //gpioDelay(300000);
            }
        }
        else  // If no streets were found
        {
            // TODO: Corrigir
            dist_to_move_mm = 0;
            std::cout << "Girando pra ver se encontra uma linha" << std::endl;
            if(turn_direction > 0)
                movement.turn(-5);
            else
                movement.turn(5);
        }       
        
        /* Take a new picture and find the tapes and streets in it */
        vision.getDownwardCamImg();
        std::tie(found_tapes, found_streets) = vision.findStreets();
        std::cout << "Ruas encontradas: " << std::endl;
        std::for_each(found_streets.begin(), found_streets.end(), [](auto sec){sec.print();});
        // Separate the tapes to the left from the tapes to the right of the vehicle
        left_tapes = std::vector<streets::StreetSection>();
        right_tapes = std::vector<streets::StreetSection>();
        std::cout << "Fitas encontradas" << std::endl;
        for (const auto& tape: found_tapes)
        {
            float angle = setAngleInRange(tape.line[1], (20*M_PI)/180);
            std::cout << tape.line << ' ' << tape.seg <<" " << " angulo eh " << angle <<std::endl;
            if(std::abs(angle) < 30)
            {
                if(tape.seg[0] < 0)  // Both seg[0] and seg[2] should give the same results
                    left_tapes.push_back(tape);
                else
                    right_tapes.push_back(tape);
            }
        }
        // Sort the tapes by their closest point
        streets::orderCollinearSections(left_tapes, 1);
        streets::orderCollinearSections(right_tapes, 1);
        std::cout << "Fitas da esquerda: " << std::endl;
        std::for_each(left_tapes.begin(), left_tapes.end(), [](auto sec){sec.print();});
        std::cout << "Fitas da direita: " << std::endl;
        std::for_each(right_tapes.begin(), right_tapes.end(), [](auto sec){sec.print();});
        /* Calculate how much was moved */
        // Search for where the tape is expected to be
        // If the tape is found use the calculated moved distance as the truth
        int num_tapes_found = 0;
        float ran_dist_step_mm = 0;
        if (left_reference_tape.color != streets::Color::none)
        {
            for(const auto& tape: left_tapes)
            {
                const float expected_pos = left_reference_tape.seg[1] - consts::step_size_mm/1000;
                if (((expected_pos-0.05) <= tape.seg[1])
                    && (tape.seg[1] <= (expected_pos+0.05)))
                {
                    std::cout << "Achou a fita da esquerda em: " << tape.seg;
                    num_tapes_found++;
                    ran_dist_step_mm += (left_reference_tape.seg[1] - tape.seg[1]);
                    break;  // Stop as there should only be one tape that fits
                }
            }
        }
        if (right_reference_tape.color != streets::Color::none)
        {
            for(const auto& tape: right_tapes)
            {
                const float expected_pos = right_reference_tape.seg[1] - dist_to_move_mm/1000;
                if (((expected_pos-0.05) <= tape.seg[1])
                    && (tape.seg[1] <= (expected_pos+0.05)))
                {
                    std::cout << "Achou a fita da direita em: " << tape.seg;
                    num_tapes_found++;
                    ran_dist_step_mm += (right_reference_tape.seg[1] - tape.seg[1]) * 1000;
                    break;
                }
            }
        }
        // If a tape is found both to the left and to the right, take the mean
        if (num_tapes_found == 2)
        {
            ran_dist_step_mm /= 2;
        }
        // If no tapes were found, assume the vehicle moved the correct amount
        else if (num_tapes_found == 0)
        {
            ran_dist_step_mm = dist_to_move_mm;
        }
        std::cout << "Acha que andou: " << ran_dist_step_mm << std::endl;
        total_ran_dist_mm += ran_dist_step_mm;

        std::cout << "No total andou: " << total_ran_dist_mm << std::endl << std::endl;
        if(total_ran_dist_mm >= required_distance)
            stop = true;
    }
    movement.stop();
    gpioTerminate();
    return 0;
}
