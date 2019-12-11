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
	//cv::Mat img = cv::imread("./teste_linhas.jpg", cv::IMREAD_COLOR);
	//cv::cvtColor(img, vision.downward_img, cv::COLOR_BGR2HLS);
	for(int i = 0; i<1; i++)
	{
		vision.getDownwardCamImg();
		cv::Mat img;
		cv::cvtColor(vision.downward_img, img, cv::COLOR_HLS2BGR);
		cv::imwrite("teste_linhas.jpg", img);
		std::cout << "Tirou a foto." << std::endl;
		auto initial_tick = cv::getTickCount();
		auto [tape_secs, street_secs] = vision.findStreets();
		auto time_to_process = (cv::getTickCount() - initial_tick) / cv::getTickFrequency();
		std::cout << "Tempo para processar: " << time_to_process << std::endl;
		
		auto tapes_img = streets::drawStreetsAndTapes(tape_secs, street_secs, true, false);
		cv::imwrite("teste_linhas_so_fita.jpg", tapes_img);
		auto tapes_streets_img = streets::drawStreetsAndTapes(tape_secs, street_secs, true, true);
		cv::imwrite("teste_linhas_secoes.jpg", tapes_streets_img);
		
		std::cout << "Ruas:" << std::endl;
		std::for_each(street_secs.begin(), street_secs.end(), [](const auto& sec){sec.print();});
		std::cout << "Fitas:" << std::endl;
		std::for_each(tape_secs.begin(), tape_secs.end(), [](const auto& sec){sec.print();});
	}
	std::cout << "buscando qr code" << std::endl;
	//cv::Mat img = cv::imread("./teste_top_img.jpg", cv::IMREAD_COLOR);
	//img.copyTo(vision.forward_img);
	vision.getForwardCamImg();
	cv::imwrite("teste_top_img.jpg", vision.forward_img);
	std::cout << "tem semafaro vermelho ? " << vision.isTrafficLightRed() << std::endl;
	auto [ids, corners] = vision.findCityARMarkers();
	for(unsigned int i=0; i < ids.size(); i++)
	{
		std::cout << "encontrou " << ids[i] << " " << corners[i] << std::endl;
		cv::rectangle(vision.forward_img, corners[i][0], corners[i][2], cv::Scalar(0,255,0),  3, cv::LINE_8, 0); 
	}
	cv::imwrite("results.jpg", vision.forward_img);
	vision.getForwardCamImg();
	cv::imwrite("teste_top_img_qr_user.jpg", vision.forward_img);
	std::tie(ids, corners) = vision.findAppARMarkers();
	for(unsigned int i=0; i < ids.size(); i++)
	{
		std::cout << "encontrou " << ids[i] << " " << corners[i] << std::endl;
	}
}
