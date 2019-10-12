#ifndef street_lines_h_
#define street_lines_h_

#include <vector>
#include <opencv2/core.hpp>

namespace street_lines
{
	void getStreetLines(const cv::Mat& lines_mask, std::vector<cv::Vec4i>& street_lines);

	float getStreetLineAngle(const cv::Vec4i& line);
	
	cv::Vec4i linePxToDist(const cv::Vec4i& line);

	float lineAbsDist(const cv::Vec4i& line);

	std::vector<int> groupLinesByAngle(std::vector<float>& lines_angles);
}

#endif
