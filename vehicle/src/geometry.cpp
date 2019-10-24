#include "geometry.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <opencv2/core.hpp>

namespace street_lines
{
    // Finds the distance between two line segments (x1, y1, x2, y2)
    // Reference: http://paulbourke.net/geometry/pointlineplane/
    float distSegments(const cv::Vec4i& seg1, const cv::Vec4i& seg2)
    {
        if (segmentsIntersect(seg1, seg2))
            return 0;
        else
        {
            return std::min({distPointSegment(cv::Point2i(seg1[0], seg1[1]), seg2),
                             distPointSegment(cv::Point2i(seg1[2], seg1[3]), seg2),
                             distPointSegment(cv::Point2i(seg2[0], seg2[1]), seg1),
                             distPointSegment(cv::Point2i(seg2[2], seg2[3]), seg1)});
        }
    }

    // Tells if the two line segments intersect or not
    bool segmentsIntersect(const cv::Vec4i& seg1, const cv::Vec4i& seg2)
    {
        // If they're the same line
        if (seg1 == line2)
            return true;

        const auto uan = (seg2[2]-seg2[0]) * (seg1[1]-seg2[1])
                          - (seg2[3]-seg2[1]) * (seg1[0]-seg2[0]);
        const auto ubn = (seg1[2]-seg1[0]) * (seg1[1]-seg2[1])
                          - (seg1[3]-seg1[1]) * (seg1[0]-seg2[0]);
        const auto denominator = (seg2[3]-seg2[1]) * (seg1[2]-seg1[0]) 
                                  - (seg2[2]-seg2[0]) * (seg1[3]-seg1[1]);

        // If they are parallel
        if (denominator == 0)
            return false;
        const auto ua = uan / float(denominator);
        const auto ub = ubn / float(denominator);
        // If they intersect
        if ((0 <= ua && ua <= 1) && (0 <= ub && ub <= 1))
            return true;
        // If they don't intersect
        else
            return false;
    }

    // Calculates the distance between a point and a line segment
    float distPointSegment(const cv::Point& pt, cv::Vec4i seg)
    {
        const auto u_num = (pt.y-seg[0]) * (seg[2]-seg[0])
                           + (pt.x-seg[1]) * (seg[3]-seg[1]);
        const auto u_den = (seg[2] - seg[0]) * (seg[2] - seg[0])
                           + (seg[3] - seg[1]) * (seg[3] - seg[1]);
        float u;
        if (u_den != 0)
            u = u_num / float(u_den);
        else
            u = std::numeric_limits<float>::infinity();

        if (0 <= u && u <= 1)
        {
            const auto x = seg[0] + u*(seg[2] - seg[0]);
            const auto y = seg[1] + u*(seg[3] - seg[1]);
            return distPoints(pt, cv::Point(x, y));
        }
        else
            return std::min(distPoints(pt, cv::Point(seg[0], seg[1])),
                            distPoints(pt, cv::Point(seg[2], seg[3])));
    }

    // Calculates the distance between two points
    float distPoints(const cv::Point& pt1, const cv::Point& pt2)
    {
        return sqrt((pt1.y-pt2.y)*(pt1.y-pt2.y)
                    + (pt1.x-pt2.x)*(pt1.x-pt2.x));
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

        return {line1[0] - u*sin(line1[1]), line1[0]*sin(line1[1]) + u*cos(line1[1])};
    }
}
