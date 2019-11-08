#include "reduce_lines.hpp"
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "geometry.hpp"
#include "constants.hpp"


using std::vector;
using cv::Vec4i;
using cv::Point2f;
using cv::Vec2f;
using constants::img_width;
using constants::img_height;
using cv::Mat;
using cv::Point;

namespace street_lines
{    
    
    Mat drawSegmentsTrash(const vector<Vec4i>& segs, const Mat& img_segs)
    {
        //Mat img_segs = img.copy();
        //cv::cvtColor(img, img_segs, cv::COLOR_HLS2BGR);
        for (auto seg: segs)
        {
            cv::line(img_segs, Point2f(seg[0], seg[1]), Point2f(seg[2], seg[3]), cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
        }
        return img_segs;
    }
    
    vector<Vec4i> reduceSegments (const vector<Vec4i>& segs)
    {
        vector<Vec2f> lines(segs.size());
        std::transform(segs.begin(), segs.end(), lines.begin(), xySegmentToLine);
        // Separate the lines by angle
        auto groups = groupLinesByAngle(lines, constants::max_theta_diff);
        // For each group, find the rotated bounding rectangle and the line that crosses it
        vector<Vec4i> out_segs;
        int a = 0;
        char name[] = "group0.png";
        for (auto group: groups)
        {
            std::cout << "group " << a <<std::endl;
            a++;
            // Ignore very small groups
            if (group.size() < 5)
                continue;
            // Find the line of the group
            /*float theta = 0;
            float rho = 0;
            int used_axis;
            if(abs(segs[group[0]][2] - segs[group[0]][0]) > abs(segs[group[0]][3] - segs[group[0]][1]))
                used_axis = 0;
            elsePoint
                used_axis = 1;
                theta
            for (auto i: group)
            {
                rho += lines[i][0];
                const float line_angle = lines[i][1];
                if (line_angle > M_PI)
                    theta += line_angle - M_PI;
                else
                    theta += line_angle;
            }
            theta /= group.size();
            rho /= group.size();
            std::cout << "angle: " << theta << std::endl;
            
            Mat img_line = Mat::zeros(img_width, img_height, CV_8U);
            drawLine(Vec2f(rho, theta), img_line);
            Vec4f seg(img_width, img_height, 0, 0);
            int used_axis;
            vector<vector<cv::Point>> contours;
            vector<Vec4i> hierarchy;
            cv::findContours(red_mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);*/
            
            
            float theta = 0;
            for (auto i: group)
            {
                const float line_angle = lines[i][1];
                if (line_angle > M_PI)
                    theta += line_angle - M_PI;
                else
                    theta += line_angle;
            }
            theta /= group.size();
            std::cout << "angle " << theta <<std::endl; 
            cv::Point2f min((img_width + img_height), (img_width + img_height));
            cv::Point2f max(-(img_width + img_height), -(img_width + img_height));
            vector<Vec4i> rotate_points;
            for (auto i: group)
            {
                const Point2f rot_pt1 = rotatePoint(Point2f(segs[i][0], segs[i][1]), -theta);
                min.x = std::min(min.x, rot_pt1.x);
                min.y = std::min(min.y, rot_pt1.y);
                max.x = std::max(max.x, rot_pt1.x);
                max.y = std::max(max.y, rot_pt1.y);
                const Point2f rot_pt2 = rotatePoint(Point2f(segs[i][2], segs[i][3]), -theta);
                min.x = std::min(min.x, rot_pt2.x);
                min.y = std::min(min.y, rot_pt2.y);
                max.x = std::max(max.x, rot_pt2.x);
                max.y = std::max(max.y, rot_pt2.y);
                rotate_points.emplace_back(rot_pt1.x, rot_pt1.y, rot_pt2.x, rot_pt2.y); 
            }
            auto line = xySegmentToLine(rotate_points[0]);
            std::cout << "rotate angle" << line[1] <<std::endl;
            for (auto point: rotate_points)
            {
                std::cout << point <<std::endl; 
            }
            Mat img_line = Mat::zeros(img_width, img_height, CV_8U);
            img_line = drawSegmentsTrash(rotate_points, img_line);
            name[5] = '0' + a;
            cv::imwrite(name, img_line);
            
            const Point2f rot_line1((min.x+max.x) / 2, min.y);
            const Point2f rot_line2((min.x+max.x) / 2, max.y);
            const Point2f unrot_line1 = rotatePoint(rot_line1, theta);
            const Point2f unrot_line2 = rotatePoint(rot_line2, theta);
            out_segs.emplace_back(unrot_line1.x, unrot_line1.y, unrot_line2.x, unrot_line2.y);
        }
        return out_segs;
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
                        if(linesAreCollinear(lines[i], lines[j], max_theta_diff, 200))//200 tape width in pixels in the image border
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
    
    // Draws the line on the image
    void drawLine(const Vec2f& line, const Mat& img)
    {
        const auto len = 2*(img.rows + img.cols);
        const float a = cos(line[1]);
        const float b = sin(line[1]);
        const float x0 = a * line[0];
        const float y0 = b * line[0];
        const Point pt1(x0 + len*(-b), y0 + len*a);
        const Point pt2(x0 - len*(-b), y0 - len*a);
        cv::line(img, pt1, pt2, 255, 1, cv::LINE_8);
    }
}
