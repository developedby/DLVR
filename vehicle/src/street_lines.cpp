#include "street_lines.hpp"

#include <limits>
#include <vector>
#include <opencv2/core.hpp>
#include <reduce_lines.hpp>

namespace street_lines
{
	void getStreetLines(const cv::Mat& lines_mask, std::vector<cv::Vec4i>& lines)
	{
		cv::HoughLinesP(mask, lines, 1, np.radians(1), 80, 20, 10);
		reduceLines(lines, lines, 0.2, 2.0, 10.0);
	}

	std::vector<float> getStreetLineAngle(const cv::Vec4i& line)
	{
		// Finds the point of intersection at the horizon
		float x_horizon;
		// Vector given by pt1 + v*(pt2 - pt1)
		if (line[3] != line[1])
		{
			float v = float(this->horizon_y - line[1]) / float(line[3] - line[1]);
			x_horizon = line[0] + v*(line[2] - line[0]);
		}
		else
			x_horizon = std::numeric_limits<float>::infinity();
		
		// Finds the angle
		float angle = std::atan2(this->img_height*(1 - this->horizon_y), (x_horizon - this->img_width/2));
		// Corrects the angle to the vehicle
		angle -= real_zero_deg;
		
		return angle;
	}
	
	cv::Vec4i linePxToDist(const cv::Vec4i& line)
	{
		
	}
}
