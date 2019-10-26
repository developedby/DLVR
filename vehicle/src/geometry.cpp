#include "geometry.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <opencv2/core.hpp>

namespace street_lines
{
    inline auto square (auto num)
        return num*num;

    inline auto norm (auto x, auto y)
        return sqrt(square(x) + square(x));

    cv::Vec2f segmentHalfPoint(const cv::Vec4f& seg)
    {
        const float a = seg[0];
        const float b = seg[3];
        const float c = sqrt(square(a) + square(b) - 2*a*b*cos(abs(seg[1]-seg[3]));
        return cv::Vec2f(sqrt((a*b)*(a+b+c)*(a+b-c)) / (a+b),
                         seg[1] + seg[3])/2);
    }

    // Finds the distance between two line segments (x1, y1, x2, y2)
    // Reference: http://paulbourke.net/geometry/pointlineplane/
    float distXYSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2)
    {
        if (xySegmentsIntersect(seg1, seg2))
            return 0;
        else
        {
            return std::min({distXYPointXYSegment(cv::Point2f(seg1[0], seg1[1]), seg2),
                             distXYPointXYSegment(cv::Point2f(seg1[2], seg1[3]), seg2),
                             distXYPointXYSegment(cv::Point2f(seg2[0], seg2[1]), seg1),
                             distXYPointXYSegment(cv::Point2f(seg2[2], seg2[3]), seg1)});
        }
    }

    // Finds the distance between two (rho1, theta1, rho2, theta2) segments
    float distRTSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2)
    {
        return distXYSegments(segmentRTToXY(seg1), segmentRTToXY(seg2));
    }

    // Tells if the two line segments intersect or not
    bool xySegmentsIntersect(const cv::Vec4f& seg1, const cv::Vec4f& seg2)
    {
        // If they're the same line
        if (seg1 == line2)
            return true;

        const float uan = (seg2[2]-seg2[0]) * (seg1[1]-seg2[1])
                          - (seg2[3]-seg2[1]) * (seg1[0]-seg2[0]);
        const float ubn = (seg1[2]-seg1[0]) * (seg1[1]-seg2[1])
                          - (seg1[3]-seg1[1]) * (seg1[0]-seg2[0]);
        const float denominator = (seg2[3]-seg2[1]) * (seg1[2]-seg1[0]) 
                                  - (seg2[2]-seg2[0]) * (seg1[3]-seg1[1]);

        // If they are parallel
        if (denominator == 0)
            return false;
        const auto ua = uan / denominator;
        const auto ub = ubn / denominator;
        // If they intersect
        if ((0 <= ua && ua <= 1) && (0 <= ub && ub <= 1))
            return true;
        // If they don't intersect
        else
            return false;
    }

    // Calculates the distance between a point and a line segment
    float distXYPointXYSegment(const cv::Point& pt, cv::Vec4f seg)
    {
        const float u_num = (pt.y-seg[0]) * (seg[2]-seg[0])
                           + (pt.x-seg[1]) * (seg[3]-seg[1]);
        const float u_den = (seg[2] - seg[0]) * (seg[2] - seg[0])
                           + (seg[3] - seg[1]) * (seg[3] - seg[1]);
        float u;
        if (u_den != 0)
            u = u_num / u_den;
        else
            u = std::numeric_limits<float>::infinity();

        if (0 <= u && u <= 1)
        {
            const float x = seg[0] + u*(seg[2] - seg[0]);
            const float y = seg[1] + u*(seg[3] - seg[1]);
            return distXYPoints(pt, cv::Point(x, y));
        }
        else
            return std::min(distXYPoints(pt, cv::Point(seg[0], seg[1])),
                            distXYPoints(pt, cv::Point(seg[2], seg[3])));
    }

    // Calculates the distance between two points
    float distXYPoints(const cv::Point& pt1, const cv::Point& pt2)
    {
        return sqrt((pt1.y-pt2.y)*(pt1.y-pt2.y)
                    + (pt1.x-pt2.x)*(pt1.x-pt2.x));
    }

    // Converts (x1, y1, x2, y2) segments to (rho, theta) lines
    void xySegmentsToLines(const std::vector<cv::Vec4f>& segs, std::vector<cv::Vec2f>& lines)
    {
        for (auto i = segs.begin(); i != segs.end(); i++)
        {
            float theta = -atan2(segs[i][0]-segs[i][2], segs[i][1]-segs[i][3]);
            if (theta > M_PI/2)
                theta -= M_PI;
            else if (theta <= -M_PI/2)
                theta += M_PI;
            float rho = abs(segs[i][2]*segs[i][1] - segs[i][3]*segs[i][0])
                            / norm(segs[i][3]-segs[i][1], segs[i][2]-segs[i][0]);
            lines[i] = Vec2f(theta, rho);
        }
    }

    // Calculates the intersection point of two (rho, theta) lines
    cv::Vec2f linesIntersection(const cv::Vec2f& line1, const cv::Vec2f& line2)
    {
        // If the lines are parallel, intersection is said to be at infinity
        if (abs(sin(line1[1]) - sin(line2[1])) < 0.01)
        {
            return {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}
        }
        // The lines can be defined as vectors '(rho*cos(theta), rho*sin(theta)) + u*(-sin(theta), cos(theta))'
        // By saying these two vectors are equal, we can isolate u
        // Using x dimensoin to find u
        const float u = (line2[0]*cos(line2[1]) - line1[0]*cos(line1[1]))
                       / (sin(line2[1]) - sin(line1[1]));
        // Using y dimension to find u (equivalent formula)
        //const auto u = (line1[0]*sin(line1[1]) - line2[0]*sin(line2[1]))
                       / (cos(line2[1]) - cos(line1[1]));

        return cv::Vec2f(line1[0] - u*sin(line1[1]), line1[0]*sin(line1[1]) + u*cos(line1[1]));
    }

    // Decides whether or not two lines make a certain angle between them
    // Considers a certain precision and that angles are always between [-M_PI/2, M_PI/2]
    bool linesHaveAngle(const cv::Vec2f& line1, const cv::Vec2f& line2, const float angle, const float max_theta_diff)
    {
        float angle_diff = abs(line[1] - line2[1]);
        if (angle_diff > M_PI/2)
            angle_diff = abs(angle_diff - M_PI);
            
        if ((angle - max_theta_diff) < angle_diff && angle_diff < (angle + max_theta_diff))
            return true;
        else
            return false;
    }

    bool linesArePerpendicular(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff)
    {
        return linesHaveAngle(line1, line2, M_PI/2, max_theta_diff);
    }

    bool linesAreParallel(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff)
    {
        return linesHaveAngle(line1, line2, 0, max_theta_diff);
    }

    // Transforms a (rho1, theta1, rho2, theta2) segment to a (x1, y1, x2, y2) segment
    cv::Vec4f segmentRTToXY(const cv::Vec4f& rt_seg)
    {
        return cv::Vec4f(rt_seg[0]*cos(rt_seg[1]), rt_seg[0]*sin(rt_seg[1]),
                         rt_seg[2]*cos(rt_seg[3]), rt_seg[2]*sin(rt_seg[3]));
    }

    // Transforms a (x1, y1, x2, y2) segment to a (rho1, theta1, rho2, theta2) segment
    cv::Vec4f segmentXYToRT(const cv::Vec4f& xy_seg)
    {
        return cv::Vec4f(norm(xy_seg[0], xy_seg[1]), atan2(xy_seg[1], xy_seg[0]),
                         norm(xy_seg[2], xy_seg[3]), atan2(xy_seg[3], xy_seg[2]));
    }
}
