#include "street_lines.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <reduce_lines.hpp>
#include "geometry.hpp"
#include "constants.hpp"

using constants::img_y_horizon;
using constants::img_height;
using constants::img_width;
using constants::img_theta_min;
using constants::dist_theta_min_m;
using constants::img_real_zero_rad;
using constants::max_theta_diff;
using std::vector;
using std::pair;
using cv::Vec4i;
using cv::Vec4f;
using cv::Vec2f;
using cv::Mat;
using cv::Scalar;
using cv::Point;

namespace street_lines
{
    // Finds the lines in the mask representing the street lane markers
    vector<Vec4i> getStreetLines(const Mat& lines_mask)
    {
        vector<Vec4i> lines;
        cv::HoughLinesP(lines_mask, lines, 1, max_theta_diff-0.01, 90, 40, 8);
        std::cout << "Linhas: " << lines.size() << std::endl;
        reduceLines(lines, lines, 0.2, 7.0, 10.0);
        return lines;
    }

    // Reverte a distorção de perspectiva
    // Retornando uma linha (rho, theta) em relação ao veiculo
    // e os dois extremos do segmento no formato (rho1, theta1, rho2, theta2)
    pair<Vec2f, Vec4f> undistortLine(const Vec4i& line)
    {
        // Undistorted segment in rho theta format
        const Vec4f undistort_seg_rt = linePxToDist(line);
        // Undistorted segment in x y format
        const Vec4f undistort_seg_xy = segmentRTToXY(undistort_seg_rt);
        std::cout << "undistort: " << undistort_seg_rt << ' ' << undistort_seg_xy << std::endl;
        return pair(xySegmentToLine(segmentRTToXY(undistort_seg_rt)), undistort_seg_rt);
    }

    // Finds the distance of the line segment's extremities to the vehicle
    // Return the two points in the (rho1, theta1, rho2, theta2) format
    Vec4f linePxToDist(const Vec4i& line)
    {
        // TODO
        float constexpr x_center = img_width / 2;
        float constexpr y_theta_min = img_height;
        float constexpr px_per_rad = (img_y_horizon - y_theta_min)/img_theta_min;
        float constexpr y_vehicle = M_PI/2 * px_per_rad; // Where 90º would be if we extended the image
        float constexpr meter_per_px = dist_theta_min_m / (y_vehicle - y_theta_min); // Using a known distance
        Vec4f line_dists;
        
        // First point
        float y_angle = (line[1] - img_y_horizon) / px_per_rad;
        float stretch_factor = tan(M_PI/2 - y_angle);
        float x_dist = (line[0] - x_center) * meter_per_px * stretch_factor;
        float y_dist = (y_vehicle - line[1]) * meter_per_px * stretch_factor;
        line_dists[1] = atan2(x_dist, y_dist) - img_real_zero_rad;
        if (line_dists[1] < 0)
            line_dists[1] += 2*M_PI;
        line_dists[0] = norm(x_dist, y_dist);

        // Second point
        y_angle = (line[3] - img_y_horizon) / px_per_rad;
        stretch_factor = tan(M_PI/2 - y_angle);
        x_dist = (line[2] - x_center) * meter_per_px * stretch_factor;
        y_dist = (y_vehicle - line[3]) * meter_per_px * stretch_factor;
        line_dists[3] = atan2(y_dist, x_dist) - img_real_zero_rad;
        if (line_dists[3] < 0)
            line_dists[3] += 2*M_PI;
        line_dists[2] = norm(y_dist, x_dist);
        return line_dists;
    }

    // Finds the line segment's angle relative to the vehicle
    // Adjusts for perspective and camera rotation
    float getStreetLineAngle(const Vec4i& line)
    {
        // Finds the point of intersection at the horizon
        // Vector given by pt1 + v*(pt2 - pt1)
        float x_horizon;
        if (line[3] != line[1])
        {
            float v = float(img_y_horizon - line[1]) / float(line[3] - line[1]);
            x_horizon = line[0] + v*(line[2] - line[0]);
        }
        // If parallel to the horizon
        else
            x_horizon = std::numeric_limits<float>::infinity();
        
        // Finds the angle
        float angle = std::atan2(img_height*(1 - img_y_horizon), (x_horizon - img_width/2));
        // Corrects the angle to the vehicle
        angle -= img_real_zero_rad;
        
        return angle;
    }

    // Finds the distance between the vehicle and a line segment
    // Assumes that the segment doesn't cross the vehicle
    float lineAbsDist(const Vec4i& line)
    {
        return std::min(sqrt(line[0]*line[0] + line[1]*line[1]),
                        sqrt(line[2]*line[2] + line[3]*line[3]));
    }

    // Groups together lines with similar angles
    // Returns a vector with the index to the members of each group
    vector<vector<unsigned int>> groupLinesByAngle(const vector<Vec2f>& lines, const float max_theta_diff)
    {
        int counter = 0;
        vector<int> classification(lines.size(), -1);
        vector<vector<unsigned int>> groups;
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            if (classification[i] == -1)
            {
                groups.push_back(vector{i});
                classification[i] = counter;
                for (unsigned int j=i+1; j < lines.size(); j++)
                {
                    // Group together if angles have difference less than max_theta_diff
                    if (classification[j] == -1)
                    {
                        float delta_theta = abs(lines[i][1] - lines[j][1]);
                        if (delta_theta > M_PI/2)
                            delta_theta = M_PI - delta_theta;
                        if (delta_theta > max_theta_diff)
                        {
                            classification[j] = counter;
                            groups[counter].push_back(j);
                        }
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // From a group of maybe repeated lines, select only the unique ones, separating them by their distance (rho)
    // Return the indexes of the unique lines in the input vector
    vector<vector<unsigned int>> groupLinesByDistance(const vector<Vec2f>& lines, const float max_rho_diff)
    {
        vector<vector<unsigned int>> groups; // Each element has a group of lines that are repeated
        vector<int> classification(lines.size(), -1); // Which group a line is part of
        int counter = 0;
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            if (classification[i] == -1)
            {
                groups.push_back(vector{i});
                classification[i] = counter;
                for (unsigned int j = i+1; j < lines.size(); j++)
                {
                    if ((classification[j] == -1)
                        && ((lines[i][0]-max_rho_diff) < lines[j][0])
                        && (lines[j][0] < (lines[i][0]+max_rho_diff)))
                    {
                        classification[j] = counter;
                        groups[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // Separate the lines in different groups of collinear lines
    vector<vector<unsigned int>> groupCollinearLines(const vector<Vec2f>& lines, const float max_theta_diff, const float max_rho_diff)
    {
        vector<int> classification(lines.size(), -1);
        vector<vector<unsigned int>> groups;
        int counter = 0;
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            if (classification[i] == -1)
            {
                classification[i] = counter;
                groups.push_back(vector{i});
                for (unsigned int j = i+1; j < lines.size(); j++)
                {
                    if (classification[j] == -1
                        && linesAreCollinear(lines[i], lines[j], max_theta_diff, max_rho_diff))
                    {
                        classification[j] = counter;
                        groups[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // Decides if two segments are the same or not
    // TODO: Calculate this in a way that doesn't always say that short segments are equal and long one aren't (length (in)dependent)
    bool rtSegmentsAreSimilar(const Vec4f& seg1, const Vec4f& seg2, const float threshold)
    {
        const auto seg1_xy = segmentRTToXY(seg1);
        const auto seg2_xy = segmentRTToXY(seg2);
        // The score for how different they are is the distance between the end points
        // Takes the min because we don't know how the end points are oriented
        const float dist = std::min((distXYPoints(cv::Point(seg1_xy[0], seg1_xy[1]), cv::Point(seg2_xy[0], seg2_xy[1]))
                                + distXYPoints(cv::Point(seg1_xy[2], seg1_xy[3]), cv::Point(seg2_xy[2], seg2_xy[3]))),
                               (distXYPoints(cv::Point(seg1_xy[0], seg1_xy[1]), cv::Point(seg2_xy[2], seg2_xy[3]))
                                + distXYPoints(cv::Point(seg1_xy[2], seg1_xy[3]), cv::Point(seg2_xy[0], seg2_xy[1]))));
        return dist < threshold;
    }

    // Does a stable in-place sort of collinear points
    void orderCollinearPoints(vector<Vec2f>& pts, const float angle)
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

    // Draw the lines on the image
    Mat drawLines(const vector<Vec4i>& lines, const Mat& img)
    {
        Mat img_lines;
        cv::cvtColor(img, img_lines, cv::COLOR_HLS2RGB);
        for (auto line: lines)
        {
            cv::line(img_lines, Point(line[0], line[1]), Point(line[2], line[3]), Scalar(0, 255, 0), 1, cv::LINE_AA);
        }
        return img_lines;
    }
}
