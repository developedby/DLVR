#include "geometry.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <opencv2/core.hpp>

namespace street_lines
{
    // Finds the distance between two lines (x1, y1, x2, y2)
    // Reference: http://paulbourke.net/geometry/pointlineplane/
    float distLines(const cv::Vec4i& line1, const cv::Vec4i& line2)
    {
        if (linesIntersect(line1, line2))
            return 0;
        else
        {
            return std::min({(distPointLine(cv::Point2i(line1[0], line1[1]), line2)),
                             (distPointLine(cv::Point2i(line1[2], line1[3]), line2)),
                             distPointLine(cv::Point2i(line2[0], line2[1]), line1),
                             distPointLine(cv::Point2i(line2[2], line2[3]), line1)});
        }
    }

    // Tells if the two line segments intersect or not
    bool linesIntersect(const cv::Vec4i& line1, const cv::Vec4i& line2)
    {
        // If they're the same line
        if (line1 == line2)
            return true;

        const auto uan = (line2[2]-line2[0]) * (line1[1]-line2[1])
                          - (line2[3]-line2[1]) * (line1[0]-line2[0]);
        const auto ubn = (line1[2]-line1[0]) * (line1[1]-line2[1])
                          - (line1[3]-line1[1]) * (line1[0]-line2[0]);
        const auto denominator = (line2[3]-line2[1]) * (line1[2]-line1[0]) 
                                  - (line2[2]-line2[0]) * (line1[3]-line1[1]);

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
    float distPointLine(const cv::Point& pt, cv::Vec4i line)
    {
        const auto u_num = (pt.y-line[0]) * (line[2]-line[0])
                           + (pt.x-line[1]) * (line[3]-line[1]);
        const auto u_den = (line[2] - line[0]) * (line[2] - line[0])
                           + (line[3] - line[1]) * (line[3] - line[1]);
        float u;
        if (u_den != 0)
            u = u_num / float(u_den);
        else
            u = std::numeric_limits<float>::infinity();

        if (0 <= u && u <= 1)
        {
            const auto x = line[0] + u*(line[2] - line[0]);
            const auto y = line[1] + u*(line[3] - line[1]);
            return distPoints(pt, cv::Point(x, y));
        }
        else
            return std::min(distPoints(pt, cv::Point(line[0], line[1])),
                            distPoints(pt, cv::Point(line[2], line[3])));
    }

    // Calculates the distance between two points
    float distPoints(const cv::Point& pt1, const cv::Point& pt2)
    {
        return sqrt((pt1.y-pt2.y)*(pt1.y-pt2.y)
                    + (pt1.x-pt2.x)*(pt1.x-pt2.x));
    }
}
