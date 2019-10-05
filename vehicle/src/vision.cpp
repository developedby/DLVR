#include "vision.hpp"
#include <cassert>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

Vision::Vision()
{
	this->cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
	this->cam.setRotation(3);
	auto success = this->cam.open();
	assert(success);
}

cv::Mat Vision::getCamImg()
{
	this->cam.grab();
	this->cam.retrieve(this->img);
	cv::cvtColor(this->img, this->img, cv::COLOR_BGR2HLS);
}

void Vision::getColorMask(cv::Mat dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max)
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

void Vision::getRedTapeMask(cv::Mat dst)
{
	this->getColorMask(dst, 170, 180, 100, 160, 90, 200);
}
