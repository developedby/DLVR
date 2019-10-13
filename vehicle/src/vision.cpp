#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "reduce_lines.hpp"
#include "constants.hpp"
#include "street_lines.hpp"

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
// Returns a graph of the StreetSections perceived
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
    cv::Mat street_lines_mask;
    cv::bitwise_and(blue_tape_mask, green_tape_mask, street_lines_mask);
    cv::bitwise_and(street_lines_mask, yellow_tape_mask, street_lines_mask);

    // Finds the lines passing through those masks
    std::vector<cv::Vec4i> lines;
    street_lines::getStreetLines(street_lines_mask, lines);

    // Finds the angle for each line
    std::vector<float> lines_angles(lines.size());
    std::transform(lines.begin(), lines.end(),
                   lines_angles.begin(), street_lines::getStreetLineAngle);
    
    // Finds the distance of both ends of each line to the vehicle
    std::vector<cv::Vec4i> lines_dists(lines.size());
    std::transform(lines.begin(), lines.end(),
                   lines_dists.begin(), street_lines::linePxToDist);

    // Group the lines together by their angle
    auto line_grouping = street_lines::groupLinesByAngle(lines_angles);


    // Contructs the StreetSection from the lines and the information about them
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
