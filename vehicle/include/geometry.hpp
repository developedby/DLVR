#ifndef geometry_hpp_
#define geometry_hpp_

#include <opencv2/core.hpp>

namespace street_lines
{
    cv::Vec2f segmentHalfPoint(const cv::Vec4f& seg);
    float distXYSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    float distRTSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    bool xySegmentsIntersect(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    float distXYPointXYSegment(const cv::Point& pt, cv::Vec4f seg);
    float distXYPoints(const cv::Point& pt1, const cv::Point& pt2);
    void xySegmentsToLines(const std::vector<cv::Vec4f>& segs, std::vector<cv::Vec2f>& lines);
    cv::Vec2f linesIntersection(const cv::Vec2f& line1, const cv::Vec2f& line2);
    bool linesHaveAngle(const cv::Vec2f& line1, const cv::Vec2f& line2, const float angle, const float max_theta_diff);
    bool linesArePerpendicular(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff);
    bool linesAreParallel(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff);
    cv::Vec4f segmentRTToXY(const cv::Vec4f& rt_seg);
    cv::Vec4f segmentXYToRT(const cv::Vec4f& xy_seg);
}

#endif
