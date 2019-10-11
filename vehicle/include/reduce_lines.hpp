#ifndef reduce_lines_h_
#define reduce_lines_h_

#include <vector>
#include <opencv2/core.hpp>

namespace street_lines
{
	void reduceLines (const std::vector<cv::Vec4i>& in_lines, std::vector<cv::Vec4i>& out_lines,
				      const float extend_ratio, const float delta_theta, const float rect_thickness);

	bool extendedBoundingRectangleLineEquivalence(const cv::Vec4i& _l1, const cv::Vec4i& _l2, float extensionLengthFraction,
                                                  float maxAngleDiff, float boundingRectangleThickness);

    cv::Vec4i extendedLine(cv::Vec4i line, double d);

    std::vector<cv::Point2i> boundingRectangleContour(cv::Vec4i line, float d);
    
    cv::Vec2d linearParameters(cv::Vec4i line);
}

#endif
