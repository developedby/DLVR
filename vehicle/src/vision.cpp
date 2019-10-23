#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <tuple>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "reduce_lines.hpp"
#include "constants.hpp"
#include "street_lines.hpp"

using constants::lane_width;
using constants::max_theta_diff;

Vision::Vision()
{
    this->cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    this->cam.setRotation(3);
    this->cam.set(cv::CAP_PROP_FRAME_WIDTH, constants::img_width);
    this->cam.set(cv::CAP_PROP_FRAME_HEIGHT, constants::img_height);
    auto success = this->cam.open();
    assert(success);
}

void Vision::getCamImg()
{
    this->cam.grab();
    this->cam.retrieve(this->img);
    cv::cvtColor(this->img, this->img, cv::COLOR_BGR2HLS);
}

// Looks at the image and finds the streets
// Returns a graph of the StreetSections identified
std::vector<street_lines::StreetSection> Vision::findStreets()
{
    // Finds the binary mask for each marker type
    cv::Mat blue_tape_mask;
    this->getBlueTapeMask(blue_tape_mask);
    cv::Mat green_tape_mask;
    this->getGreenTapeMask(green_tape_mask);
    cv::Mat yellow_tape_mask;
    this->getYellowTapeMask(yellow_tape_mask);
    
    // Combines the masks
    //cv::Mat street_lines_mask;
    //cv::bitwise_and(blue_tape_mask, green_tape_mask, street_lines_mask);
    //cv::bitwise_and(street_lines_mask, yellow_tape_mask, street_lines_mask);

    // Finds the lines passing through those masks
    std::vector<cv::Vec4i> lines;
    
    enum class Colors {blue, green, yellow, red};
    std::vector<Colors> line_colors;
    
    auto lines_aux = street_lines::getStreetLines(blue_tape_mask);
    lines.insert(line.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(lines_aux.size(), Colors.blue);
    
    std::vector<cv::Vec2f> undistort_lines(lines.size());
    std::vector<cv::Vec4f> undistort_segs(lines.size());
    for (int i = lines.begin(); i != lines.end(); i++)
    {
        std::tie(undistort_lines[i], undistort_segs[i] = street_lines::undistortLine(lines[i]);
    }

    // Group the lines together by their angle
    auto line_grouping = street_lines::groupLinesByAngle(undistort_lines, max_theta_diff);

    // Find the street(s) that passes through each group
    std::vector<cv::Vec2f> streets;
    for (auto group: line_grouping)
    {
        // Separate each group into some lines
        std::vector<std::vector<int>> some_indexes = street_lines::groupLinesByDistance(group, lane_width/2);
        
        // Infer where the streets should be
        // If there's only one line, assume the street to be in the direction of the vehicle
        if (some_indexes.size() == 1)
        {
            auto street = undistort_lines[group[some_indexes[0]]];
            // Offset in the vehicle's direction
            street[0] -= constants::lane_width / 2;
            // If the distance is negative, invert the angle
            if (street[0] < 0)
            {
                street[0] = -street[0].
                if (street[1] > 0)
                    street[1] -= M_PI;
                else
                    street[1] += M_PI;
            }
            streets.push_back(undistort_lines[group[some_indexes[0]]]);
            
        }
    }
    
    // Find the intersections between the streets

    // Contructs the StreetSection graph from the lines and the information about them


    //Takes the nearest line as the current section
    //float i_min_dist = std::min_element(street_lines_dists.begin(), street_lines_dists.end(),
      //                                  street_lines::lineAbsDist);


    //return ;//
}

void Vision::getColorMask(cv::Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max)
{
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), dst);
}

void Vision::getRedTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 160, 180, 100, 240, 40, 140);
}

void Vision::getBlueTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 100, 120, 70, 200, 25, 170);
}

void Vision::getGreenTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 60, 90, 70, 150, 30, 120);
}

void Vision::getYellowTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 20, 40, 130, 190, 30, 170);
}

void Vision::getWhiteTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 20, 110, 120, 255, 0, 70);
}

void Vision::getGroundMask(cv::Mat& dst)
{
    this->getColorMask(dst, 0, 180, 50, 120, 0, 30);
}

bool Vision::isTrafficLightRed()
{
    cv::Mat red_mask;
    getRedTapeMask(red_mask);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(red_mask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    if(hierarchy.size()>0)
    {
        for(int j=0; j>=0; j=hierarchy[j][0])
        {
            double area = cv::contourArea(contours[j]);
            if(area < 20)
            {
                return true;
            }
        }
    }
    return false;
    
}