#ifndef geometry_hpp_
#define geometry_hpp_

#include <opencv2/core.hpp>

namespace street_lines
{
    float distLines(const cv::Vec4i& line1, const cv::Vec4i& line2);
    bool linesIntersect(const cv::Vec4i& line1, const cv::Vec4i& line2);
    float distPointLine(const cv::Point& pt, cv::Vec4i& line);
    float distPoints(const cv::Point& pt1, const cv::Point& pt2);
}

#endif