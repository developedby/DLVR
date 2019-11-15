#ifndef reduce_lines_h_
#define reduce_lines_h_

#include <vector>
#include <opencv2/core.hpp>

namespace street_lines
{
	std::vector<cv::Vec4i> reduceSegments (const std::vector<cv::Vec4i>& segs, const cv::Mat& label_image, const int n_labels);
	std::vector<std::vector<unsigned int>> groupLinesByAngle(const std::vector<cv::Vec2f>& lines, const float max_theta_diff);
	void drawBlobs(const cv::Mat& labelImage, int nLabels);
}

#endif
