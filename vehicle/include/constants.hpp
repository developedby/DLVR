#ifndef constants_h_
#define constants_h_

namespace constants
{
	// Vision
	int constexpr img_height = 960;
	int constexpr img_width = 1280;
	float constexpr img_y_horizon = 0.282;
	float constexpr img_real_zero_deg = 4.647;
	float constexpr img_y_vehicle = 1.;
	float constexpr img_theta_min = 29.12;

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
}

#endif
