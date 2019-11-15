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
    
    vector<Vec4i> reduceSegments (const vector<Vec4i>& segs, const Mat& label_image, const int n_labels)
    {
        //std::cout << "\nreduceSegments com " << segs.size() << " segs" << std::endl;
        // Separate lines by blob
        if (constants::save_img)
            drawBlobs(label_image, n_labels);
        vector<vector<Vec4i>> segs_by_blob(n_labels);
        for (auto& seg: segs)
        {
            const int label_1 = label_image.at<int>(seg[1], seg[0]);
            const int label_2 = label_image.at<int>(seg[3], seg[2]);
            if ((label_1 == label_2)
                || (label_1 != 0  && label_2 == 0)
                || (label_2 != 0  && label_1 == 0))
            {
                (segs_by_blob[label_1 ? label_1 : label_2]).push_back(seg);
            }
        }

        vector<Vec4i> out_segs;
        for (auto blob: segs_by_blob)
        {
            //std::cout << "esse blob tem tamanho " << blob.size() << std::endl;
            vector<Vec2f> lines(blob.size());
            std::transform(blob.begin(), blob.end(), lines.begin(), xySegmentToLine);
            auto groups = groupLinesByAngle(lines, 2*constants::max_theta_diff);
            int a = 0;
            for (auto group: groups)
            {         
                //std::cout << "group " << a<< " " <<group.size()<<std::endl;
                a++;
                // Ignore very small groups
                if (group.size() < 10)
                    continue;

                float theta = 0;
                vector<cv::Vec2f> seg_pts;
                for (auto i: group)
                {
                    seg_pts.emplace_back(blob[i][0], blob[i][1]);
                    seg_pts.emplace_back(blob[i][2], blob[i][3]);
                    const float line_angle = lines[i][1];
                    if (line_angle > M_PI)
                        theta += line_angle - M_PI;
                    else
                        theta += line_angle;
                }
                theta /= group.size();
                //std::cout << "angle " << theta <<std::endl;
                orderCollinearPoints(seg_pts, theta);
                
                Vec4i result_seg(0, 0, 0, 0);
                const unsigned int n_pts = seg_pts.size();
                float constexpr percentile = 0.3;
                for (int i = 0; i < int(percentile*n_pts); i++)
                {
                    result_seg[0] += seg_pts[i][0];
                    result_seg[1] += seg_pts[i][1];
                    result_seg[2] += seg_pts[n_pts-i-1][0];
                    result_seg[3] += seg_pts[n_pts-i-1][1];
                }
                result_seg[0] /= int(percentile*n_pts);
                result_seg[1] /= int(percentile*n_pts);
                result_seg[2] /= int(percentile*n_pts);
                result_seg[3] /= int(percentile*n_pts);
                out_segs.push_back(result_seg);
            }
        }
        //std::cout << "reduce lines" <<std::endl;
        //for (auto seg:out_segs)
            //std::cout << seg <<std::endl;
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
    
    void drawBlobs(const cv::Mat& labelImage, int nLabels)
    {
        std::vector<cv::Vec3b> colors(nLabels);
        colors[0] = cv::Vec3b(0, 0, 0);//background
        for(int label = 1; label < nLabels; ++label)
        {
            colors[label] = cv::Vec3b( (rand()&255), (rand()&255), (rand()&255) );
        }
        cv::Mat dst(labelImage.size(), CV_8UC3);
        for(int r = 0; r < dst.rows; ++r)
        {
            for(int c = 0; c < dst.cols; ++c)
            {
                int label = labelImage.at<int>(r, c);
                cv::Vec3b &pixel = dst.at<cv::Vec3b>(r, c);
                pixel = colors[label];
            }
        }
        cv::imwrite( "blobs.jpg", dst);
    }
}
