#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "reduce_lines.hpp"
#include "constants.hpp"

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

std::vector<StreetSection> Vision::findStreets()
{
	cv::Mat blue_tape_mask;
	this->getBlueTapeMask(blue_tape_mask);
	cv::Mat green_tape_mask;
	this->getGreenTapeMask(green_tape_mask);
	cv::Mat yellow_tape_mask;
	this->getYellowTapeMask(yellow_tape_mask);
	
	cv::Mat street_lines_mask;
	cv::bitwise_and(blue_tape_mask, green_tape_mask, street_lines_mask);
	cv::bitwise_and(street_lines_mask, yellow_tape_mask, street_lines_mask);
	std::vector<cv::Vec4i> street_lines;
	street_lines::getStreetLines(street_lines_mask, street_lines);
	
	std::vector<float> street_lines_angles;
	angles.resize(street_lines.size());
	std::transform(street_lines.begin(), street_lines.end(), street_lines_angles.begin(), street_lines::getStreetLineAngle);
	
	std::vector<std::pair<float>> street_lines_dists;
	street_lines_dists.resize(street_lines.size());
	std::transform(street_lines.begin(), street_lines.end(), street_lines_dists.begin()
}

void Vision::getColorMask(cv::Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max)
{
	cv::Mat img_ch[3];
	cv::split(this->img, img_ch);
	cv::Mat mask_aux;
	cv::inRange(img_ch[0], h_min, h_max, mask)
	cv::inRange(img_ch[1], l_min, l_max, mask_aux);
	cv::bitwise_and(mask, mask_aux, mask);
	cv::inRange(img_ch[2], s_min, s_max, mask_aux);
	cv::bitwise_and(mask, mask_aux, mask);	
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
