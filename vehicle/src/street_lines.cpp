#include "street_lines.hpp"
#include <cmath>
#include <limits>
#include <vector>
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
using constants::img_real_zero_deg;

namespace street_lines
{
    inline auto squared (auto num)
        return num*num;
        
    inline auto norm (auto x, auto y)
        return sqrt(squared(x) + squared(x));
    
    // Finds the lines in the mask representing the street lane markers
    std::vector<cv::Vec4i> getStreetLines(const cv::Mat& lines_mask)
    {
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(lines_mask, lines, 1, M_PI/180, 80, 20, 10);
        reduceLines(lines, lines, 0.2, 2.0, 10.0);
        return lines;
    }

    // Reverte a distorção de perspectiva
    // Retornando uma linha (rho, theta) em relação ao veiculo
    // e os dois extremos do segmento no formato (rho1, theta1, rho2, theta2)
    std::pair<cv::Vec2f, cv::Vec4f> undistortLine(const cv::Vec4i& line)
    {
        // Undistorted segment in rho theta format
        const cv::Vec4f undistort_seg_rt = linePxToDist(line);
        // Undistorted segment in x y format
        const cv::Vec4f undistort_seg_xy(undistort_seg_rt[0] * sin(undistort_seg_rt[1]),
                                         undistort_seg_rt[0] * cos(undistort_seg_rt[1]),
                                         undistort_seg_rt[2] * sin(undistort_seg_rt[3]),
                                         undistort_seg_rt[2] * cos(undistort_seg_rt[3]));

        const float u_num = (0-line[0]) * (line[2]-line[0])
                             + (0-line[1]) * (line[3]-line[1]);
        const float u_den = (line[2] - line[0]) * (line[2] - line[0])
                             + (line[3] - line[1]) * (line[3] - line[1]);
        const float u = u_num / u_den;
        const float x = line[0] + u*(line[2] - line[0]);
        const float y = line[1] + u*(line[3] - line[1]);
        
        return std::pair(cv::Vec2f(norm(x, y), atan2(x, y)), dist_extremities);
    }

    // Finds the line segment's angle relative to the vehicle
    // Adjusts for perspective and camera rotation
    float getStreetLineAngle(const cv::Vec4i& line)
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
        angle -= img_real_zero_deg;
        
        return angle;
    }
    
    // Finds the distance of the line segment's extremities to the vehicle
    // Return the two point in the (rho1, theta1, rho2, theta2) format
    cv::Vec4f linePxToDist(const cv::Vec4i& line)
    {
        float constexpr x_center = img_width / 2;
        float constexpr y_theta_min = img_height;
        float constexpr px_per_rad = (img_y_horizon - y_theta_min)/img_theta_min;
        float constexpr y_vehicle = 90 * px_per_rad; // Where 90º would be if we extended the image
        float constexpr dist_theta_min_m = 0.145; // Distance to the bottom point of the image
        float constexpr meter_per_px = dist_theta_min_m / (y_vehicle - y_theta_min); // Using a known distance
        cv::Vec4i line_dists;
        
        // First point
        float y_angle = (line[1] - img_y_horizon) / px_per_rad;
        float stretch_factor = tan(M_PI/2 - y_angle);
        float x_dist = (line[0] - x_center) * meter_per_px * stretch_factor;
        float y_dist = (line[1] - img_y_horizon) * meter_per_px * stretch_factor;
        line_dists[1] = atan2(x_dist, y_dist) - img_real_zero_rad;
        line_dists[0] = norm(x_dist, y_dist);

        // Second point
        y_angle = (line[3] - img_y_horizon) / px_per_rad;
        stretch_factor = tan(M_PI/2 - y_angle);
        x_dist = (line[2] - x_center) * meter_per_px * stretch_factor;
        y_dist = (line[3] - img_y_horizon) * meter_per_px * stretch_factor;
        line_dists[3] = atan2(x_dist, y_dist) - img_real_zero_rad;
        line_dists[2] = norm(x_dist, y_dist);
        return line_dists;
    }

    float lineAbsDist(const cv::Vec4i& line)
    {
        return std::min(sqrt(line[0]*line[0] + line[1]*line[1]),
                        sqrt(line[2]*line[2] + line[3]*line[3]));
    }
    
    // Groups together lines with similar angles
    // Returns a vector with the index to the members of each group
    std::vector<std::vector<int>> groupLinesByAngle(const std::vector<cv::Vec2f>& lines, const float max_theta_diff)
    {
        int group_counter = 0;
        std::vector<int> line_grouping(lines.size(), -1);
        std::vector<std::vector<int>> groups;
        for (auto i=lines.begin(); i != lines.end(); i++)
        {
            if (line_grouping[i] == -1)
            {
                groups.push_back(std::vector(i));
                line_grouping[i] = group_counter;
                for (auto j=i+1; j != lines.end()); j++)
                {
                    // Group together if angles have difference less than max_theta_diff
                    if ((line_grouping[j] == -1)
                        && ((lines[i][1]-max_theta_diff) < lines[j][1])
                        && (lines[j][1] < (lines[i][1]+max_theta_diff)))
                    {
                        lines_angles[j] = group_counter;
                        groups[group_counter].push_back(j);
                    }
                }
                group_counter++;
            }
        }
        return groups;
    }
    
    // From a group of maybe repeated lines, select only the unique ones, separating them by their distance (rho)
    // Return the indexes of the unique lines
    std::vector<std::vector<int>> groupLinesByDistance(const std::vector<cv::Vec2f>& lines, const float max_rho_diff)
    {
        std::vector<std::vector<int>> groups_of_unique_lines; // Each element has a group of lines that are repeated
        std::vector<int> classification(lines.size(), -1); // Which group a line is part of
        int counter = 0;
        for (auto i = lines.begin(); i != lines.end(); i++)
        {
            if (classification[i] == -1)
            {
                unique_lines_index.push_back({i});
                classification[i] = counter;
                for (auto j = i+1; j != lines.end(); j++)
                {
                    if ((classification[j] == -1)
                        && ((lines[i][0]-max_rho_diff) < lines[j][0])
                        && (lines[j][0] < (lines[i][0]+max_rho_diff)))
                    {
                        classification[j] = counter;
                        groups_of_unique_lines[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return unique_lines_index;
    }

    // Converts (x1, y1, x2, y2) segments to (rho, theta) lines
    void segmentsToLines(const std::vector<cv::Vec4i>& segs, std::vector<cv::Vec2f>& lines)
    {
        for (auto i = segs.begin(); i != segs.end(); i++)
        {
            auto theta = -atan2(segs[i][0]-segs[i][2], segs[i][1]-segs[i][3]);
            if (theta > M_PI/2)
                theta -= M_PI;
            else if (theta <= -M_PI/2)
                theta += M_PI;
            auto rho = abs(segs[i][2]*segs[i][1] - segs[i][3]*segs[i][0])
                           / norm(segs[i][3]-segs[i][1], segs[i][2]-segs[i][0]);
            lines[i] = Vec2f(theta, rho);
        }
    }
}
