#include "street_lines.hpp"
#include <cmath>
#include <limits>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <reduce_lines.hpp>
#include "constants.hpp"

using constants::img_y_horizon;
using constants::img_height;
using constants::img_width;
using constants::img_theta_min;
using constants::img_real_zero_deg;

namespace street_lines
{
	void getStreetLines(const cv::Mat& lines_mask, std::vector<cv::Vec4i>& lines)
	{
		cv::HoughLinesP(lines_mask, lines, 1, (M_PI / 180), 80, 20, 10);
		reduceLines(lines, lines, 0.2, 2.0, 10.0);
	}

	// Given a (x1, y1, x2, y2) line, find it's angle relative to the vehicle
	// Adjusts for perspective and camera rotation
	float getStreetLineAngle(const cv::Vec4i& line)
	{
		// Finds the point of intersection at the horizon
		float x_horizon;
		// Vector given by pt1 + v*(pt2 - pt1)
		if (line[3] != line[1])
		{
			float v = float(img_y_horizon - line[1]) / float(line[3] - line[1]);
			x_horizon = line[0] + v*(line[2] - line[0]);
		}
		else
			x_horizon = std::numeric_limits<float>::infinity();
		
		// Finds the angle
		float angle = std::atan2(img_height*(1 - img_y_horizon), (x_horizon - img_width/2));
		// Corrects the angle to the vehicle
		angle -= img_real_zero_deg;
		
		return angle;
	}
	
	// Given a (x1, y1, x2, y2) line from an image, calculates the distance of those points to the vehicle
	cv::Vec4i linePxToDist(const cv::Vec4i& line)
	{
		float constexpr x_center = img_width / 2;
		float constexpr y_theta_min = img_height;
		float constexpr px_per_rad = (img_y_horizon - img_height)/img_theta_min;
		float constexpr y_vehicle = 90 * px_per_rad;
		float constexpr dist_theta_min_m = 0.145; // Distance to the bottom point of the image
		float constexpr meter_per_px = dist_theta_min_m / (y_vehicle - img_height); // Using a known distance

		cv::Vec4i line_dists;
		float y_angle = (line[1] - img_y_horizon) / px_per_rad;
		float stretch_factor = tan(M_PI/2 - y_angle);
		line_dists[0] = (line[0] - x_center) * meter_per_px * stretch_factor;
		line_dists[1] = (line[1] - img_y_horizon) * meter_per_px * stretch_factor;

		y_angle = (line[3] - img_y_horizon) / px_per_rad;
		stretch_factor = tan(M_PI/2 - y_angle);
		line_dists[2] = (line[2] - x_center) * meter_per_px * stretch_factor;
		line_dists[3] = (line[3] - img_y_horizon) * meter_per_px * stretch_factor;
		return line_dists;
	}
}
