#include <iostream>
#include <vector>
#include <pigpio.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "vision.hpp"
#include "streets.hpp"
#include "geometry.hpp"

int main()
{
	if (gpioInitialise() == PI_INIT_FAILED)
    {
        std::cout << "Erro ao inicializar a GPIO!" << std::endl;
        exit(PI_INIT_FAILED);
    }
	Vision vision = Vision();
	//cv::Mat img = cv::imread("./img/linha_verde.jpg", cv::IMREAD_COLOR);
	//cv::cvtColor(img, vision.downward_img, cv::COLOR_BGR2HLS);
	/*for(int i = 0; i< 2; i++)
	{
		vision.getDownwardCamImg();
		cv::Mat img;
		cv::cvtColor(vision.downward_img, img, cv::COLOR_HLS2BGR);
		cv::imwrite("teste_linhas.jpg", img);
		//std::cout << "Tirou a foto." << std::endl;
		auto initial_tick = cv::getTickCount();
		auto [tape_secs, street_secs] = vision.findStreets();
		auto time_to_process = (cv::getTickCount() - initial_tick) / cv::getTickFrequency();
		std::cout << "Tempo para processar: " << time_to_process << std::endl;
		
		cv::Mat out_img = cv::Mat::zeros(500, 500, CV_8UC3);
		for (const auto& sec: street_secs)
		{
			cv::Vec4f line = sec.seg;
			line[0] = 250 + line[0]*500;
			line[1] = 500 - line[1]*1000;
			line[2] = 250 + line[2]*500;
			line[3] = 500 - line[3]*1000;
			cv::line(out_img, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(255, 255, 255), 5, cv::LINE_AA);
			sec.print();
		}
		for (const auto& sec: tape_secs)
		{
			cv::Vec4f line = sec.seg;
			line[0] = 250 + line[0]*500;
			line[1] = 500 - line[1]*1000;
			line[2] = 250 + line[2]*500;
			line[3] = 500 - line[3]*1000;
			cv::Scalar color;
			if (sec.color == streets::Color::blue)
				color = cv::Scalar(255, 200, 65);
			else if (sec.color == streets::Color::green)
				color = cv::Scalar(0, 255, 0);
			else if (sec.color == streets::Color::yellow)
				color = cv::Scalar(0, 255, 255);
			else
				color = cv::Scalar(0, 0, 255);
			cv::line(out_img, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 5, cv::LINE_AA);
			sec.print();
		}
		
		cv::imwrite("teste_linhas_secoes.jpg", out_img);
		
	}*/
	vision.getForwardCamImg();
	cv::imwrite("teste_top_img.jpg", vision.forward_img);
	auto [ids, corners] = vision.findARMarkers();
	for(unsigned int i=0; i < ids.size(); i++)
	{
		std::cout << "encontrou " << ids[i] << " " << corners[i] << std::endl;
		cv::rectangle(vision.forward_img, corners[i][0], corners[i][2], cv::Scalar(0,255,0),  3, cv::LINE_8, 0); 
	}
	cv::imwrite("results.jpg", vision.forward_img);
}
