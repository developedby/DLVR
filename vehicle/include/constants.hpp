#ifndef constants_h_
#define constants_h_

#include <cstdint>
#include <cmath>

namespace consts
{
	// Vision
	// Street finder
	int constexpr img_height = 960;
	int constexpr img_width = 1280;
	float constexpr cam_height_m = 0.128;
	float constexpr img_theta_min_m = 0.062;
	float constexpr img_theta_min = atan2(cam_height_m, img_theta_min_m);
	float constexpr img_theta_max_m = 0.470;
	float constexpr img_theta_max = atan2(cam_height_m, img_theta_max_m);
	float constexpr lane_width = 0.26;
	float constexpr tape_width = 0.018;
	float constexpr max_theta_diff = 20 * M_PI/180;
	float constexpr hough_precision_px = 1;
	float constexpr hough_precision_rad = 5 * M_PI/180;
	int constexpr hough_thresh = 90;
	int constexpr hough_min_len = 90;
	int constexpr hough_max_gap = 8;
	bool constexpr save_img = true;
	// Traffic light finder
	int constexpr max_traffic_light_area = 250;
	int constexpr min_traffic_light_area = 30;

	// Radio
	int constexpr radio_delay_transmitting = 0;
	int constexpr radio_retries = 3;
	int constexpr radio_width_address = 5;
	int constexpr radio_width_data = 45;
	uint8_t constexpr radio_start_byte = 0xaa;
	uint8_t constexpr radio_address[] = "DLVR1";
	uint8_t constexpr coord_address[] = "DLVRC";
	int constexpr time_out_radio_ack = 1000000;
	int constexpr radio_pooling_period = 300000;

	// Pins
	int constexpr item_detector_pin1 = 2;
	int constexpr item_detector_pin2 = 5;
	int constexpr servo_pin = 13;
	int constexpr left_motor_pin_fwd = 7;
	int constexpr left_motor_pin_bkwd = 17;
	int constexpr left_motor_pin_pwm = 12;
	int constexpr left_encoder_pin = 26;
	int constexpr right_motor_pin_fwd = 21;
	int constexpr right_motor_pin_bkwd = 20;
	int constexpr right_motor_pin_pwm = 18;
	int constexpr right_encoder_pin = 3;
	int constexpr ultrasound_trigger_pin = 27;
	int constexpr ultrasound_echo_pin = 22;
	
	// Movement
	float constexpr turn_speed = 1.0;
	
	enum class WheelType {left, right};
	
	//vehicle const
	int constexpr vehicle_id = 1;
	int constexpr num_holes_encoder = 20;
	float constexpr wheel_radius_mm = 33.2;
	float constexpr wheel_circumference_mm = 2*M_PI*wheel_radius_mm;
	float constexpr encoder_deg_per_hole = 360 / num_holes_encoder;
	float constexpr mm_moved_per_hole = wheel_circumference_mm / num_holes_encoder;
	float constexpr wheel_distance = 128.9;
	float constexpr vehicle_deg_per_mm = 360 / (2*M_PI*wheel_distance);
	float constexpr vehicle_deg_per_hole = mm_moved_per_hole * vehicle_deg_per_mm;
	
	//pid
	float constexpr pid_l_Kp = 0.00004;
	float constexpr pid_l_Ki = 0.00162;
	float constexpr pid_l_Kd = 0.0;
	float constexpr pid_r_Kp = 0.00005;
	float constexpr pid_r_Ki = 0.0018;
	float constexpr pid_r_Kd = 0.0;
	float constexpr pid_T_ms = 100;
	
	// Intelligence
	float constexpr turn_angle_threshold = 5.0;
	float constexpr step_size_mm = 100;
	float constexpr dist_to_look_perpendicular_street = 160;
	float constexpr dist_to_avoid_distance_cm = 6;
}

#endif
