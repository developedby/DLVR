#include <iostream>
#include <vector>
#include <pigpio.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "vision.hpp"

int main()
{
	Vision vision = Vision();
	//vision.getCamImg();
	cv::Mat img = cv::imread("./img/linha_verde.jpg", cv::IMREAD_COLOR);
	cv::cvtColor(img, vision.img, cv::COLOR_BGR2HLS);
	//cv::Mat img;
	cv::cvtColor(vision.img, img, cv::COLOR_HLS2BGR);
	cv::imwrite("teste_linhas.jpg", img);
	std::cout << "Tirou a foto." << std::endl;
	auto sections = vision.findStreets();
	for (auto sec: sections)
	{
		std::cout << int(sec.type) << ' ' << sec.line << ' '  << sec.end_points << std::endl;
	}
}
