#ifndef street_lines_h_
#define street_lines_h_

#include <vector>
#include <tuple>
#include <opencv2/core.hpp>

namespace street_lines
{
	std::vector<cv::Vec4i> getStreetLines(const cv::Mat& lines_mask);
	
	cv::Vec4f imgSegToRealSeg(const cv::Vec4i& line);

	float lineAbsDist(const cv::Vec4i& line);
	
	std::pair<cv::Vec2f, cv::Vec4f> undistortLine(const cv::Vec4i& line);
	
	std::vector<std::vector<unsigned int>> groupLinesByDistance(const std::vector<cv::Vec2f>& lines, const float max_rho_diff);
	
	std::vector<std::vector<unsigned int>> groupCollinearLines(const std::vector<cv::Vec2f>& lines, const float max_theta_diff, const float max_rho_diff);
	
	bool rtSegmentsAreSimilar(const cv::Vec4f& seg1, const cv::Vec4f& seg2, const float threshold);
	
	cv::Mat drawSegments(const std::vector<cv::Vec4i>& lines, const cv::Mat& img);
}

#endif
