#include <iostream>
#include <vector>
#include <pigpio.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "vision.hpp"
#include "street_finder.hpp"
#include "geometry.hpp"

int main()
{
	Vision vision = Vision();
	//cv::Mat img = cv::imread("./img/linha_verde.jpg", cv::IMREAD_COLOR);
	//cv::cvtColor(img, vision.downward_img, cv::COLOR_BGR2HLS);
	vision.getDownwardCamImg();
	cv::Mat img;
	cv::cvtColor(vision.downward_img, img, cv::COLOR_HLS2BGR);
	cv::imwrite("teste_linhas.jpg", img);
	//std::cout << "Tirou a foto." << std::endl;
	auto initial_tick = cv::getTickCount();
	std::vector<street_finder::StreetSection> sections = vision.findStreets();
	auto time_to_process = (cv::getTickCount() - initial_tick) / cv::getTickFrequency();
	std::cout << "Tempo para processar: " << time_to_process << std::endl;
	
	cv::Mat out_img = cv::Mat::zeros(100, 100, CV_8U);
	for (auto& sec: sections)
	{
		cv::Vec4f line = sec.seg * 100;
		line[1] = -line[1];
		line[3] = -line[3];
		line += cv::Vec4f(50, 50, 50, 50);
		cv::line(out_img, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), 255, 1, cv::LINE_AA);
		std::cout << int(sec.color) << ' ' << sec.line << ' ' << sec.seg << std::endl;
	}
	cv::imwrite("teste_linhas_secoes.jpg", out_img);
	
	
	vision.getForwardCamImg();
	//cv::imwrite("teste_top_img.jpg", vision.top_img);
	auto [ids, corners] = vision.findARMarkers();
	for(unsigned int i=0; i < ids.size(); i++)
	{
		std::cout << "encontrou " << ids[i] << " " << corners[i] << std::endl;
		cv::rectangle(vision.forward_img, corners[i][0], corners[i][2], cv::Scalar(0,255,0),  3, cv::LINE_8, 0); 
	}
	//cv::imwrite("results.jpg", vision.top_img);
}
