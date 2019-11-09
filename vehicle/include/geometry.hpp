#ifndef geometry_hpp_
#define geometry_hpp_

#include <cmath>
#include <opencv2/core.hpp>

namespace street_lines
{
    inline float square (float num)
    {
        return num*num;
    }
        
    inline float norm (float x, float y)
    {
        return sqrt(square(x) + square(y));
    }
    
    inline float my_atan2(float y, float x)
    {
        float angle = atan2(y, x);
        if (angle < 0)
            angle += 2*M_PI;
        return angle;
    }
    
    cv::Vec2f xySegmentHalfPoint(const cv::Vec4f& seg);
    
    cv::Vec2f rtSegmentHalfPoint(const cv::Vec4f& seg);
    
    float distXYSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    
    float distRTSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    
    bool xySegmentsIntersect(const cv::Vec4f& seg1, const cv::Vec4f& seg2);
    
    float distXYPointXYSegment(const cv::Point2f& pt, cv::Vec4f seg);
    
    float distXYPoints(const cv::Point2f& pt1, const cv::Point2f& pt2);
    
    cv::Vec2f xySegmentToLine(const cv::Vec4f& seg);
    
    cv::Vec2f linesIntersection(const cv::Vec2f& line1, const cv::Vec2f& line2);
    
    bool linesHaveAngle(const cv::Vec2f& line1, const cv::Vec2f& line2, const float angle, const float max_theta_diff);
    
    bool linesArePerpendicular(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff);
    
    bool linesAreParallel(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff);

    bool linesAreCollinear(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff, const float max_rho_diff);
    
    cv::Vec4f segmentRTToXY(const cv::Vec4f& rt_seg);
    
    cv::Vec4f segmentXYToRT(const cv::Vec4f& xy_seg);
    
    cv::Point2f rotatePoint(const cv::Point2f pt, const float angle);
    
    void orderCollinearPoints(std::vector<cv::Vec2f>& pts, const float angle);
}

#endif
