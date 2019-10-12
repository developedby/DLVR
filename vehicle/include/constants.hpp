#ifndef constants_h_
#define constants_h_

#include<cstdint>

namespace constants
{
	int const img_height = 960;
	int const img_width = 1280;
	float const img_y_horizon = 0.282;
	float const img_real_zero_deg = 4.647;
	float const img_y_vehicle = 1.;
	float const img_theta_min = 29.12;
	int const item_detector_pin1 = 2;
	int const item_detector_pin2 = 5;
	int const servo_pin = 13;
	int const left_motor_pin_fwd = 7;
	int const left_motor_pin_bkwd = 17;
	int const left_motor_pin_pwm = 12;
	int const left_encoder_pin = 26;
	int const right_motor_pin_fwd = 21;
	int const right_motor_pin_bkwd = 20;
	int const right_motor_pin_pwm = 18;
	int const right_encoder_pin = 3;
	int const ultrasound_trigger_pin = 27;
	int const ultrasound_echo_pin = 22;
	int const radio_delay_transmitting = 0;
	int const radio_retries = 3;
	int const radio_width_address = 5;
	int const radio_width_data = 45;
	uint8_t const radio_start_byte = 0xaa;
	uint8_t const radio_address[] = "DLVR1";
	uint8_t const coord_address[] = "DLVRC";
	int const time_out_radio_ack = 500000;
}

#endif
