#include "street_lines.hpp"
#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>
#include <opencv2/core.hpp>
#include <reduce_lines.hpp>
#include "geometry.hpp"

using constants::y_horizon, constants::img_height,
      constants::img_width, constants::img_theta_min, constants::img_real_zero_deg;

namespace street_lines
{
    // Finds the lines in the mask representing the street lane markers
    void getStreetLines(const cv::Mat& lines_mask, std::vector<cv::Vec4i>& lines)
    {
        cv::HoughLinesP(mask, lines, 1, np.radians(1), 80, 20, 10);
        reduceLines(lines, lines, 0.2, 2.0, 10.0);
    }

    // Finds the line segment's angle relative to the vehicle
    // Adjusts for perspective and camera rotation
    std::vector<float> getStreetLineAngle(const cv::Vec4i& line)
    {
        // Finds the point of intersection at the horizon
        // Vector given by pt1 + v*(pt2 - pt1)
        float x_horizon;
        if (line[3] != line[1])
        {
            float v = float(y_horizon - line[1]) / float(line[3] - line[1]);
            x_horizon = line[0] + v*(line[2] - line[0]);
        }
        // If parallel to the horizon
        else
            x_horizon = std::numeric_limits<float>::infinity();
        
        // Finds the angle
        float angle = std::atan2(img_height*(1 - y_horizon), (x_horizon - img_width/2));
        // Corrects the angle to the vehicle
        angle -= img_real_zero_deg;
        
        return angle;
    }
    
    // Finds the distance of the line segment's extremities to the vehicle
    cv::Vec4i linePxToDist(const cv::Vec4i& line)
    {
        float constexpr x_center = img_width / 2;
        float constexpr y_theta_min = img_height;
        float constexpr px_per_rad = (img_y_horizon - y_theta_min)/img_theta_min;
        float constexpr y_vehicle = 90 * px_per_rad;
        float constexpr dist_theta_min_m = 0.145; // Distance to the bottom point of the image
        float constexpr meter_per_px = dist_theta_min / (y_vehicle - y_theta_min); // Using a known distance

        cv::Vec4i line_dists;
        float y_angle = (line[1] - img_y_horizon) / px_per_rad;
        float stretch_factor = tan(M_PI/2 - y_angle);
        line_dists[0] = (line[0] - x_center) * meter_per_px * stretch_factor;
        line_dists[1] = (line[1] - img_y_horizon) * meter_per_px * stretch_factor;

        y_angle = (line[3] - img_y_horizon) / px_per_rad;
        stretch_factor = tan(M_PI/2 - y_angle);
        line_dists[2] = (line[2] - x_center) * meter_per_px * stretch_factor;
        line_dists[3] = (line[3] - img_y_horizon) * meter_per_px * stretch_factor;
        return line_dists;
    }

    float lineAbsDist(const cv::Vec4i& line)
    {
        return std::min(sqrt(line[0]*line[0] + line[1]*line[1]),
                        sqrt(line[2]*line[2] + line[3]*line[3]));
    }

    std::vector<int> groupLinesByAngle(const std::vector<cv::Vec4i>& lines_angles)
    {
        int group_counter = 0;
        std::vector<int> line_grouping(lines_angles.size(), -1);
        for (auto i=lines_angles.begin(); i != lines_angles.end(); i++)
        {
            if (line_grouping[i] == -1)
            {
                line_grouping[i] = group_counter;
                for (auto j=i+1; j != lines_angles.end(); j++)
                {
                    // Group together if angles have difference less than [-5, 5]
                    if (line_grouping[j] == -1
                        && lines_angles[i]-5 < lines_angles[j]
                        && lines_angles[j] < lines_angles[i]+5)
                    {
                        lines_angles[j] = group_counter;
                    }
                }
                group_counter++;
            }
        }
        return line_grouping;
    }
}
