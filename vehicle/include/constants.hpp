#ifndef constants_h_
#define constants_h_

#include<cstdint>

namespace constants
{
	// Vision
	int constexpr img_height = 960;
	int constexpr img_width = 1280;
	float constexpr img_y_horizon = 0.282;
	float constexpr img_real_zero_deg = 4.647;
	float constexpr img_y_vehicle = 1.;
	float constexpr img_theta_min = 29.12;

	// Radio
	int constexpr radio_delay_transmitting = 0;
	int constexpr radio_retries = 3;
	int constexpr radio_width_address = 5;
	int constexpr radio_width_data = 45;
	uint8_t constexpr radio_start_byte = 0xaa;
	uint8_t constexpr radio_address[] = "DLVR1";
	uint8_t constexpr coord_address[] = "DLVRC";
	int constexpr time_out_radio_ack = 500000;

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
	
	//vehicle const
	int constexpr vehicle_id = 1;
	float constexpr vehicle_wheel_distance = 128.9;
	
	//pid
	float constexpr pid_l_Kp = 0.00004;
	float constexpr pid_l_Ki = 0.00162;
	float constexpr pid_l_Kd = 0.0;
	float constexpr pid_r_Kp = 0.00005;
	float constexpr pid_r_Ki = 0.0018;
	float constexpr pid_r_Kd = 0.0;
	float constexpr pid_T_ms = 100;
}

#endif
