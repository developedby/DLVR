#include "geometry.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <opencv2/core.hpp>

namespace geometry
{
    float segmentLength(const cv::Vec4f& seg)
    {
        return norm(seg[2]-seg[0], seg[3]-seg[1]);
    }
    
    cv::Vec2f segmentHalfPoint(const cv::Vec4f& seg)
    {
        return cv::Vec2f((seg[0]+seg[2])/2, (seg[1]+seg[3])/2);
    }

    // Finds the distance between two line segments (x1, y1, x2, y2)
    // Reference: http://paulbourke.net/geometry/pointlineplane/
    float distSegments(const cv::Vec4f& seg1, const cv::Vec4f& seg2)
    {
        if (segmentsIntersect(seg1, seg2))
            return 0;
        else
        {
            return std::min({distPointSegment(cv::Point2f(seg1[0], seg1[1]), seg2),
                             distPointSegment(cv::Point2f(seg1[2], seg1[3]), seg2),
                             distPointSegment(cv::Point2f(seg2[0], seg2[1]), seg1),
                             distPointSegment(cv::Point2f(seg2[2], seg2[3]), seg1)});
        }
    }

    // Tells if the two line segments intersect or not
    bool segmentsIntersect(const cv::Vec4f& seg1, const cv::Vec4f& seg2)
    {
        // If they're the same line
        if (seg1 == seg2)
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
    float distPointSegment(const cv::Point2f& pt, cv::Vec4f seg)
    {
        const float u_num = (pt.x-seg[0]) * (seg[2]-seg[0])
                            + (pt.y-seg[1]) * (seg[3]-seg[1]);
        const float u_den = square(seg[2]-seg[0]) + square(seg[3]-seg[1]);
        float u;
        if (u_den != 0)
            u = u_num / u_den;
        else //it's a point
            return (distPoints(pt, cv::Point2f(seg[0], seg[1])));

        if (0 <= u && u <= 1)
        {
            const float x = seg[0] + u*(seg[2] - seg[0]);
            const float y = seg[1] + u*(seg[3] - seg[1]);
            return distPoints(pt, cv::Point2f(x, y));
        }
        else
            return std::min(distPoints(pt, cv::Point2f(seg[0], seg[1])),
                            distPoints(pt, cv::Point2f(seg[2], seg[3])));
    }

    // Calculates the distance between two points
    float distPoints(const cv::Point2f& pt1, const cv::Point2f& pt2)
    {
        return norm(pt1.y-pt2.y, pt1.x-pt2.x);
    }

    // Converts a (x1, y1, x2, y2) segment to a (rho, theta) line
    cv::Vec2f segmentToLine(const cv::Vec4f& seg)
    {
        float line_theta = atan2((seg[3] - seg[1]), (seg[2] - seg[0]));
        if (line_theta < 0)
            line_theta += M_PI;
        const float x1 = seg[0];
        const float y1 = seg[1];
        const float x2 = 0;
        const float y2 = 0;
        const float theta1 = line_theta;
        const float theta2 = line_theta - M_PI/2;
        const float u = ((y1-y2) + (x2-x1)*tan(theta1))
                         / (sin(theta2) - tan(theta1)*cos(theta2));
        const float x_intersect = x2 + u*cos(theta2);
        const float y_intersect = y2 + u*sin(theta2);
        //std::cout << "Convertendo segmento pra linha: " << seg << " -> " << cv::Vec2f(x_intersect, y_intersect) << std::endl;
        const float rho = norm(x_intersect, y_intersect);
        const float theta = my_atan2(y_intersect, x_intersect);
        return cv::Vec2f(rho, theta);
    }

    // Calculates the intersection point of two (rho, theta) lines
    cv::Vec2f linesIntersection(const cv::Vec2f& line1, const cv::Vec2f& line2)
    {
        // If the lines are parallel, intersection is said to be at infinity
        if (linesAreParallel(line1, line2, 0.01))
        {
            //std::cout << line1 << "eh paralelo com " << line2 << std::endl;
            return cv::Vec2f(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
        }
        
        // (x1, y1) + u1(cos(theta1) + sin(theta1)) = (x2, y2) + u2(cos(theta2), sin(theta2))
        // x1 + u1*cos(theta1) = x2 + u2*cos(theta2)
        // y1 + u1*sin(theta1) = y2 + u2*sin(theta2)
        const float x1 = line1[0] * cos(line1[1]);
        const float y1 = line1[0] * sin(line1[1]);
        const float x2 = line2[0] * cos(line2[1]);
        const float y2 = line2[0] * sin(line2[1]);
        const float theta1 = line1[1] + M_PI/2;
        const float theta2 = line2[1] + M_PI/2;
        // isolating u1, substituting it into the second equation and isolating u2
        const float u2 = ((y1-y2) + (x2-x1)*tan(theta1)) / (sin(theta2) - tan(theta1)*cos(theta2));
        // Applying u2 back to the original equation, we find the intersection point
        return cv::Vec2f(x2 + u2*cos(theta2), y2 + u2*sin(theta2));
    }

    // Decides whether or not two lines make a certain angle between them
    // Considers a certain precision and that angles are always between [-M_PI/2, M_PI/2]
    bool linesHaveAngle(const cv::Vec2f& line1, const cv::Vec2f& line2, const float angle, const float max_theta_diff)
    {
        float angle_diff = std::abs(line1[1] - line2[1]);
        if(angle_diff > 3*M_PI_2)
            angle_diff = std::abs(angle_diff - 2*M_PI);
        else if (angle_diff > M_PI/2)
            angle_diff = std::abs(angle_diff - M_PI);
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

    bool linesAreCollinear(const cv::Vec2f& line1, const cv::Vec2f& line2, const float max_theta_diff, const float max_rho_diff)
    {
        const bool lines_are_parallel = linesAreParallel(line1, line2, max_theta_diff);
        const float theta_diff = std::abs(line1[1] - line2[1]);
        bool lines_are_close;
        if (M_PI/2 < theta_diff && theta_diff < 3*M_PI/2)
            lines_are_close = std::abs(line1[0] + line2[0]) <= max_rho_diff;
        else
            lines_are_close = std::abs(line1[0] - line2[0]) <= max_rho_diff;
        //std:: cout << "linhas sao paralelas: " << lines_are_parallel << " linhas sao pertas: " << lines_are_close <<std::endl;
        return (lines_are_parallel && lines_are_close);
    }
    
    // Does a stable in-place sort of collinear points
    void orderCollinearPoints(std::vector<cv::Vec2f>& pts, const float angle)
    {
        int used_axis;
        if ((angle < M_PI/4)
            || (((M_PI - M_PI/4) < angle) && (angle < (M_PI - M_PI/4)))
            || (angle > (2*M_PI - M_PI/4)))
            used_axis = 1;
        else
            used_axis = 0;
        std::stable_sort(pts.begin(), pts.end(), [used_axis](auto pt1, auto pt2){return pt1[used_axis] < pt2[used_axis];});
    }
}
