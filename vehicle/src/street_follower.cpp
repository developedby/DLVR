#include "street_follower.hpp"
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <pigpio.h>
#include "geometry.hpp"
#include "constants.hpp"
#include "vehicle.hpp"
#include "message.hpp"
#include "movement.hpp"

StreetFollower::StreetFollower(Vehicle* vehicle_, std::vector<uint8_t> path_to_follow_, uint16_t target_qr_code_):
	vehicle(vehicle_), path_to_follow(path_to_follow_), crnt_sec(0), following_road(false), current_status(status::NO_STATUS), target_qr_code(target_qr_code_)
{
}

void StreetFollower::followPath()
{
	if (not this->path_to_follow.empty())
	{
		if (not this->following_road)
		{
			std::cout << "andando " << this->path_to_follow[2*this->crnt_sec + 1]*10 << std::endl;
			switch(Directions(this->path_to_follow[2 * this->crnt_sec]))
			{
				case FORWARD:
					break;
				case BACKWARD:
					vehicle->movement.turn(90);
					gpioSleep(PI_TIME_RELATIVE, 0, 200000);
					vehicle->movement.turn(90);
					break;
				case TO_THE_LEFT:
					vehicle->movement.turn(90);
					break;
				case TO_THE_RIGHT:
					vehicle->movement.turn(-90);
					break;
				default:
					break;
			}
			this->following_road = true;
			this->followTheRoadInit();
			this->required_dist = path_to_follow[2*this->crnt_sec + 1]*10;
			this->current_status = status::MOVING_FORWARD;
		}
		const bool road_end = this->followTheRoad();
		if (road_end)
		{
			this->following_road = false;
			this->crnt_sec++;
			if (2*this->crnt_sec == this->path_to_follow.size())
			{
				this->goToCityQrCode();
				this->path_to_follow = std::vector<uint8_t>();
				this->current_status = status::WAITING_ROUTE;
				std::cout << "terminei o caminho, status eh " << this->current_status << std::endl;;
			}
		}
	}
}

/*
 *  Choose the next point
 * Choose the tape to calculate the moved distance
 * If there isn't a obstacle or the traffic light isn't red move to next point
 * Take a new picture
 * Calculate how much was moved
 * Correct the movement
 */
bool StreetFollower::followTheRoad()
{	
	/* Choose the next point to aim for */
	// Separate streets that are parallel from streets that are perpendicular to the vehicle
	std::vector<streets::StreetSection> parallel_streets;
	std::vector<streets::StreetSection> perpendicular_streets;
	//std::cout << "escolhendo o ponto para seguir" << std::endl;
	for (const auto& street: this->found_streets)
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
	const cv::Vec2f next_pt = cv::Vec2f(avg_seg[0] + (avg_seg[2]-avg_seg[0])/2,
										avg_seg[1] + (avg_seg[3]-avg_seg[1])/2);
	const float angle = atan2(next_pt[1], next_pt[0])*(180/M_PI) - 90;
	//std::cout << "Proximo ponto a seguir: " << next_pt << " Angulo: " << angle << " deg" << std::endl;
	//std::cout << "Proxima rua: " << avg_line << std::endl;
		   
	/* Select the next tape to use to calculate the moved distance */
	// Choose the second closest tape, if available
	streets::StreetSection left_reference_tape = streets::StreetSection();
	streets::StreetSection right_reference_tape = streets::StreetSection();
	if (this->left_tapes.size() >= 2)
		left_reference_tape = this->left_tapes[1];
	else if (this->left_tapes.size() == 1)
		left_reference_tape = this->left_tapes[0];
	else
		left_reference_tape.color = streets::Color::none;
	if (this->right_tapes.size() >= 2)
		right_reference_tape = this->right_tapes[1];
	else if (this->right_tapes.size() == 1)
		right_reference_tape = this->right_tapes[0];
	else
		right_reference_tape.color = streets::Color::none;
	//std::cout << "Escolheu para calcular o quanto andou as fitas:" << std::endl;
	//std::cout << "\t" << left_reference_tape.as_str() << std::endl;
	//std::cout << "\t" << right_reference_tape.as_str() << std::endl;
	
	/* Go to the chosen point, adjusting the angle to be parallel with the street in the end */
	float dist_to_move_mm;
	if((num_pts) or ((this->required_dist - this->total_ran_dist_mm) < consts::dist_to_look_perpendicular_street))//or ((this->total_ran_dist_mm == 0) and ((this->required_dist - this->total_ran_dist_mm) < consts::dist_to_look_perpendicular_street)))
	{
		// Turn towards the chosen point
		//std::cout << required_dist - total_ran_dist_mm << "eh maior que " << consts::dist_to_look_perpendicular_street << std::endl;
		dist_to_move_mm = consts::step_size_mm;
		if((this->required_dist - this->total_ran_dist_mm) < consts::dist_to_look_perpendicular_street)
		{
			const float remaining_distace = this->required_dist - this->total_ran_dist_mm;
			float difference_point_to_stop = 100;
			for (const auto& street: perpendicular_streets)
			{
				float diff = std::abs((std::max(street.seg[1], street.seg[3])  * 1000) - remaining_distace);
				if(diff < difference_point_to_stop)
				{                        
					dist_to_move_mm = (std::min(street.seg[1], street.seg[3])  * 1000);
					std::cout << "uma rua oferece uma distancia de " << dist_to_move_mm << std::endl;
					difference_point_to_stop = diff;
				}
			}
			if((dist_to_move_mm == consts::step_size_mm) || (dist_to_move_mm > (this->required_dist - this->total_ran_dist_mm)) || (dist_to_move_mm < (this->required_dist - this->total_ran_dist_mm)*0.7))
			{
				dist_to_move_mm = (this->required_dist - this->total_ran_dist_mm)*0.75;
			}
			/*if((dist_to_move_mm > (this->required_dist - this->total_ran_dist_mm)*1.2) || (dist_to_move_mm < (this->required_dist - this->total_ran_dist_mm)*0.8))
			{
				dist_to_move_mm = (this->required_dist - this->total_ran_dist_mm);
			}*/
			std::cout << "rua perpendicular, vou andar " << dist_to_move_mm << std::endl;
			this->stop = true;
		}
		
		//check for obstacle or red traffic light
		
		/*if((vehicle->vision.distanceFromObstacle() - consts::dist_to_avoid_distance_cm) < dist_to_move_mm/10)
		{
			std::cout << "tem um obstáculo próximo: " << vehicle->vision.distanceFromObstacle() << std::endl;
			dist_to_move_mm = (vehicle->vision.distanceFromObstacle() - consts::dist_to_avoid_distance_cm) * 10;
		}*/
		if(vehicle->vision.distanceFromObstacle() < (consts::dist_to_avoid_distance_cm * 1.2))
		{
			std::cout << "tem um obstáculo próximo: " << vehicle->vision.distanceFromObstacle() << std::endl;
			this->current_status = status::AVOIDING_OBSTACLE;
			dist_to_move_mm = 0;
		}
		/*if(vehicle->vision.isTrafficLightRed())
		{
			cv::imwrite("semafaro.jpg", vehicle->vision.forward_img);
			std::cout << "semafaro fechado" << std::endl;
			dist_to_move_mm = 0;
		}*/
		//std::cout << "Angulo ate o ponto: " << angle/2 << " deg" << std::endl;
		if (std::abs(angle/2) > consts::turn_angle_threshold)
		{
			this->vehicle->movement.turn(angle/2);
			this->turn_direction = angle > 0 ? 1 : -1;
			gpioSleep(PI_TIME_RELATIVE, 0, 200000);
		}
		// Move forwards
		const float move_diff = vehicle->movement.goStraightMm(1, dist_to_move_mm, 200);
		//std::cout << "Diferença de movimento das rodas: " << move_diff << " mm" << std::endl;
		gpioSleep(PI_TIME_RELATIVE, 0, 200000);
		// Correct the error on move forward
		if (move_diff > 0)
		{
			vehicle->movement.turnOneWheel(consts::WheelType::right, 2, std::abs(move_diff));
			this->turn_direction = 1;
		}
		else if(move_diff < 0)
		{
			vehicle->movement.turnOneWheel(consts::WheelType::left, 2, std::abs(move_diff));
			this->turn_direction = -1;
		}
		gpioDelay(100000);
		// Move torwards the direction of the street
		float const correction_angle = -(angle/2 - avg_line[1])/2;
		//std::cout << "Angulo de correção: " << correction_angle << " deg" << std::endl;
		if(std::abs(correction_angle) > consts::turn_angle_threshold)
		{
			this->vehicle->movement.turn(correction_angle);
			this->turn_direction = (correction_angle > 0) ? 1 : -1;
			//gpioDelay(300000);
		}
	}
	else  // If no streets were found
	{
		// TODO: Corrigir
		dist_to_move_mm = 0;
		//std::cout << "Girando pra ver se encontra uma linha" << std::endl;
		if(this->turn_direction > 0)
			this->vehicle->movement.turn(-5);
		else
			this->vehicle->movement.turn(5);
	}       
	
	/* Take a new picture and find the tapes and streets in it */
	this->vehicle->vision.getDownwardCamImg();
	gpioSleep(PI_TIME_RELATIVE, 0, 100000); 
	std::tie(this->found_tapes, this->found_streets) = this->vehicle->vision.findStreets();
	//std::cout << "Ruas encontradas:" << std::endl;
	//std::for_each(found_streets.begin(), found_streets.end(), [](auto sec){sec.print();});
	// Separate the tapes to the left from the tapes to the right of the vehicle
	this->left_tapes = std::vector<streets::StreetSection>();
	this->right_tapes = std::vector<streets::StreetSection>();
	//std::cout << "Fitas encontradas:" << std::endl;
	for (const auto& tape: found_tapes)
	{
		const float tape_angle = setAngleInRange(tape.line[1], (20*M_PI)/180);
		//std::cout << tape.line << ' ' << tape.seg << " Angulo " << tape_angle << " deg" << std::endl;
		if(std::abs(tape_angle) < 30)
		{
			if(tape.seg[0] < 0)  // Both seg[0] and seg[2] should give the same results
				this->left_tapes.push_back(tape);
			else
				this->right_tapes.push_back(tape);
		}
	}
	// Sort the tapes by their closest point
	streets::orderCollinearSections(this->left_tapes, 1);
	streets::orderCollinearSections(this->right_tapes, 1);
	//std::cout << "Fitas da esquerda: " << std::endl;
	//std::for_each(left_tapes.begin(), left_tapes.end(), [](auto sec){sec.print();});
	//std::cout << "Fitas da direita: " << std::endl;
	//std::for_each(right_tapes.begin(), right_tapes.end(), [](auto sec){sec.print();});
	/* Calculate how much was moved */
	// Search for where the tape is expected to be
	// If the tape is found use the calculated moved distance as the truth
	int num_tapes_found = 0;
	float ran_dist_step_mm = 0;
	if (left_reference_tape.color != streets::Color::none)
	{
		const float left_reference_tape_dist = std::max(left_reference_tape.seg[1], left_reference_tape.seg[3]);
		const float expected_pos = left_reference_tape_dist - dist_to_move_mm/1000;
		for(const auto& tape: left_tapes)
		{
			float left_tape_dist = std::max(tape.seg[1], tape.seg[3]);
			if (((expected_pos-0.05) <= left_tape_dist)
				&& (left_tape_dist <= (expected_pos+0.05)))
			{
				//std::cout << "Achou a fita da esquerda em: " << tape.seg << std::endl;
				num_tapes_found++;
				ran_dist_step_mm += (left_reference_tape_dist - left_tape_dist) * 1000;
				break;  // Stop as there should only be one tape that fits
			}
		}
	}
	if (right_reference_tape.color != streets::Color::none)
	{
		const float right_reference_tape_dist = std::max(right_reference_tape.seg[1], right_reference_tape.seg[3]);
		const float expected_pos = right_reference_tape_dist - dist_to_move_mm/1000;
		for(const auto& tape: right_tapes)
		{
			float right_tape_dist = std::max(tape.seg[1], tape.seg[3]);
			if (((expected_pos-0.05) <= right_tape_dist)
				&& (right_tape_dist <= (expected_pos+0.05)))
			{
				//std::cout << "Achou a fita da direita em: " << tape.seg << std::endl;
				num_tapes_found++;
				ran_dist_step_mm += (right_reference_tape_dist - right_tape_dist) * 1000;
				break;  // Stop as there should only be one tape that fits
			}
		}
	}
	// If a tape is found both to the left and to the right, take the mean
	if (num_tapes_found == 2)
	{
		ran_dist_step_mm /= 2;
	}
	// If no tapes were found, assume the vehicle moved the correct amount
	// TODO: Algo melhor, verificar se o carro andou ou não, etc
	else if ((num_tapes_found == 0) || ran_dist_step_mm < (dist_to_move_mm*0.8) || ran_dist_step_mm > (dist_to_move_mm*1.2))
	{
		ran_dist_step_mm = dist_to_move_mm;
	}
	std::cout << "Acha que andou: " << ran_dist_step_mm << std::endl;
	this->total_ran_dist_mm += ran_dist_step_mm ;

	std::cout << "No total andou: " << this->total_ran_dist_mm << std::endl << std::endl;
	
	if(this->total_ran_dist_mm >= this->required_dist)
		this->stop = true;
    return this->stop;
}

void StreetFollower::followTheRoadInit()
{
	this->vehicle->vision.getDownwardCamImg();
    //gpioSleep(PI_TIME_RELATIVE, 0, 100000);
    std::tie(this->found_tapes, this->found_streets) = this->vehicle->vision.findStreets();
    //std::cout << "Ruas encontradas: " << std::endl;
    //std::for_each(found_streets.begin(), found_streets.end(), [](auto sec){sec.print();});
    //std::cout << "Fitas encontradas:" << std::endl;
    for (const auto& tape: found_tapes)
    {
        const float tape_angle = setAngleInRange(tape.line[1], (20*M_PI)/180);
        //std::cout << tape.line << ' ' << tape.seg << " Angulo " << tape_angle << " deg" << std::endl;
        if(std::abs(tape_angle) < 40)
        {
            if(tape.seg[0] < 0)  // Both seg[0] and seg[2] should give the same results
                this->left_tapes.push_back(tape);
            else
                this->right_tapes.push_back(tape);
        }
    }
    // Sort the tapes by their closest point
    streets::orderCollinearSections(this->left_tapes, 1);
    streets::orderCollinearSections(this->right_tapes, 1);
    
    this->total_ran_dist_mm = 0;
    this->stop = false;
    this->turn_direction = 1;
    //std::cout << std::endl;
}

bool StreetFollower::hasRoute()
{
	return not this->path_to_follow.empty();
}

void StreetFollower::goToCityQrCode()
{
    bool found = false;
    while (not found)
    {
        this->vehicle->vision.getForwardCamImg();
        const auto [ids, positions] = vehicle->vision.findCityARMarkers();
        uint i;
        for(i=0; i < ids.size(); i++)
        {
            std::cout << "encontrou " << ids[i] << " " << positions[i] << std::endl;
            if(ids[i] == this->target_qr_code)
            {
                found = true;
                break;
            }
        }
        if (not found)
        {
            vehicle->movement.turn(20);
            gpioSleep(PI_TIME_RELATIVE, 0, 400000);
		}
        else
        {
			auto mean_point = (positions[i][0] + positions[i][2]);
			mean_point.x /= 2;
			mean_point.y /= 2;
			mean_point.x -= 640/2;
			mean_point.y = 480/2 - mean_point.y;
			constexpr float pixels_by_angle = 640 / 120;
			const float angle = -(mean_point.x) / pixels_by_angle;
			float distance_to_move = (vehicle->vision.distanceFromObstacle() - consts::dist_to_avoid_distance_cm) / 2;
			std::cout << "ponto: " << mean_point.x << " angle: " << angle << " distance: " << distance_to_move <<std::endl;
			this->vehicle->movement.turn(angle/2);
			gpioSleep(PI_TIME_RELATIVE, 0, 200000);
			this->vehicle->movement.goStraightMm(1, distance_to_move*10, 200);
			gpioSleep(PI_TIME_RELATIVE, 0, 200000);
			this->vehicle->movement.turn(-angle/2);
			gpioSleep(PI_TIME_RELATIVE, 0, 200000);
			distance_to_move = std::clamp((vehicle->vision.distanceFromObstacle() - consts::dist_to_avoid_distance_cm), 0.0f, distance_to_move/2);
			if(distance_to_move > 0)
			{
				vehicle->movement.goStraightMm(1, distance_to_move*10, 200);
			}
			this->vehicle->movement.stop();
		}
    }
}

float StreetFollower::setAngleInRange(const float angle, const float precision)
{
    float result_angle = angle;
    if (result_angle > M_PI)
    {
        //std::cout << "angle " << angle <<" eh maior que " << M_PI <<" entao agora ele eh " << angle - 2*M_PI <<std::endl;
        result_angle -= 2*M_PI;
    }
    if (std::abs(result_angle) > (M_PI-precision))
    {
        //std::cout << "angle " << angle << " eh maior que a pricisao entao agora ele eh " << M_PI - std::abs(angle) <<std::endl;
        result_angle = M_PI - std::abs(result_angle);
    }
    return result_angle * (180/M_PI);
}
