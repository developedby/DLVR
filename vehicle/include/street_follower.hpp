#ifndef street_follower_hpp_
#define street_follower_hpp_

#include <vector>
#include "streets.hpp"
#include "message.hpp"

class Vehicle;

class StreetFollower
{
		Vehicle* vehicle;
		std::vector<uint8_t> path_to_follow;
		uint crnt_sec;
		std::vector<streets::StreetSection> found_tapes;
		std::vector<streets::StreetSection> found_streets;
		std::vector<streets::StreetSection> left_tapes;
		std::vector<streets::StreetSection> right_tapes;
		float required_dist;
		float total_ran_dist_mm;
		bool stop;
		int turn_direction;
		bool following_road;
		bool followTheRoad();
		void followTheRoadInit();
		void goToCityQrCode();
		float setAngleInRange(const float angle, const float precision);
		
	public:
		StreetFollower(Vehicle* vehicle_, std::vector<uint8_t> path_to_follow_, uint16_t target_qr_code_);
		void followPath();
		bool hasRoute();
		status::Status current_status;
		uint16_t target_qr_code;
};

#endif
