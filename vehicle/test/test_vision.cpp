#include <iostream>
#include <vector>
#include <pigpio.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "vision.hpp"
#include "geometry.hpp"

int main()
{
	Vision vision = Vision();
	//cv::Mat img = cv::imread("./img/linha_verde.jpg", cv::IMREAD_COLOR);
	//cv::cvtColor(img, vision.img, cv::COLOR_BGR2HLS);
	vision.getCamImg();
	cv::Mat img;
	cv::cvtColor(vision.img, img, cv::COLOR_HLS2BGR);
	cv::imwrite("teste_linhas.jpg", img);
	std::cout << "Tirou a foto." << std::endl;
	auto initial_tick = cv::getTickCount();
	auto sections = vision.findStreets();
	auto time_to_process = (cv::getTickCount() - initial_tick) / cv::getTickFrequency();
	std::cout << "Tempo para processar: " << time_to_process << std::endl;
	for (auto sec: sections)
	{
		std::cout << int(sec.type) << ' ' << sec.line << ' '  << sec.end_points << std::endl;
	}
}
