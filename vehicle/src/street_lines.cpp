#include "street_lines.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "reduce_lines.hpp"
#include "geometry.hpp"
#include "constants.hpp"

using constants::img_y_horizon;
using constants::img_height;
using constants::img_width;
using constants::cam_height_m;
using constants::img_theta_max;
using constants::img_theta_min;
using constants::img_real_zero_rad;
using constants::max_theta_diff;
using std::vector;
using std::pair;
using cv::Vec4i;
using cv::Vec4f;
using cv::Vec2f;
using cv::Mat;
using cv::Scalar;
using cv::Point2f;

namespace street_lines
{
    // Finds the lines in the mask representing the street lane markers
    vector<Vec4i> getStreetLines(const Mat& lines_mask)
    {
        vector<Vec4i> lines;
        cv::HoughLinesP(lines_mask, lines, 1, max_theta_diff-0.01, 100, 100, 8);
        //cv::imwrite("teste_linha_ruffles.jpg", drawSegments(lines, Mat::zeros(img_height, img_width, CV_8UC3)));
        //std::cout << "Linhas: " << lines.size() << std::endl;
        lines = reduceSegments(lines);
        return lines;
    }

    // Reverte a distorção de perspectiva
    // Retornando uma linha (rho, theta) em relação ao veiculo
    // e os dois extremos do segmento no formato (x1, y1, x2, y2)
    pair<Vec2f, Vec4f> undistortLine(const Vec4i& line)
    {
        // Undistorted segment in rho theta format
        const Vec4f undistort_seg = imgSegToRealSeg(line);
        //std::cout << "undistort: " << undistort_seg << std::endl;
        return pair(xySegmentToLine(undistort_seg), undistort_seg);
    }

    // Finds the distance of the line segment's extremities to the vehicle
    // Return the two points in the (x1, y1, x2, y2) format, in meters
    Vec4f imgSegToRealSeg(const Vec4i& line)
    {
        float constexpr x_center = img_width / 2;
        // Theta max e menor que theta min porque é theta da distancia maxima (mais perto do horizonte)
        float constexpr px_per_rad = img_height/(img_theta_min - img_theta_max);
        
        const float theta1 = line[1]/px_per_rad + img_theta_max;
        const float y1_m = cam_height_m / tan(theta1);
        const float phi1 = (line[0] - x_center) / px_per_rad;
        const float x1_m = tan(phi1) * y1_m;
        
        const float theta2 = line[3]/px_per_rad + img_theta_max;
        const float y2_m = cam_height_m / tan(theta2);
        const float phi2 = (line[2] - x_center) / px_per_rad;
        const float x2_m = tan(phi2) * y2_m;
        
        return Vec4f(x1_m, y1_m, x2_m, y2_m);
    }

    // Finds the distance between the vehicle and a line segment
    // Assumes that the segment doesn't cross the vehicle
    float lineAbsDist(const Vec4i& line)
    {
        return std::min(sqrt(line[0]*line[0] + line[1]*line[1]),
                        sqrt(line[2]*line[2] + line[3]*line[3]));
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
        const float dist = std::min((distXYPoints(cv::Point2f(seg1_xy[0], seg1_xy[1]), cv::Point2f(seg2_xy[0], seg2_xy[1]))
                                     + distXYPoints(cv::Point2f(seg1_xy[2], seg1_xy[3]), cv::Point2f(seg2_xy[2], seg2_xy[3]))),
                                    (distXYPoints(cv::Point2f(seg1_xy[0], seg1_xy[1]), cv::Point2f(seg2_xy[2], seg2_xy[3]))
                                     + distXYPoints(cv::Point2f(seg1_xy[2], seg1_xy[3]), cv::Point2f(seg2_xy[0], seg2_xy[1]))));
        return dist < threshold;
    }


    // Returns a bgr version of the image with the segments drawn on it
    Mat drawSegments(const vector<Vec4i>& segs, const Mat& img)
    {
        Mat img_segs;
        cv::cvtColor(img, img_segs, cv::COLOR_HLS2BGR);
        for (auto seg: segs)
        {
            cv::line(img_segs, Point2f(seg[0], seg[1]), Point2f(seg[2], seg[3]), Scalar(0, 255, 0), 1, cv::LINE_AA);
        }
        return img_segs;
    }
}
