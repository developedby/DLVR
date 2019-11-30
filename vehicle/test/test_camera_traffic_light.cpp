#include <iostream>
#include <cmath>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


int main()
{
    const std::string path =  "./img/webcam/";
    const std::string img_in = "semaforos_longe";
    const std::string extension = ".jpg";

    auto img = cv::imread(path + img_in + extension);
    cv::Mat trafic_draw;
    if (img.data == NULL)
    {
        printf("File not found");
        return -1;
    }
    img.copyTo(trafic_draw);
    cv::cvtColor(img, img, cv::COLOR_BGR2HLS);

    auto h_min = 0;
    auto h_max = 30;
    auto l_min = 100;
    auto l_max = 230;
    auto s_min = 130;
    auto s_max = 255;
    
    cv::Mat red_mask;
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), red_mask);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::imwrite("./mask.bmp", red_mask);
    cv::findContours(red_mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    if(hierarchy.size()>0)
    {
        for(int j=0; j>=0; j=hierarchy[j][0])
        {
            double area = cv::contourArea(contours[j]);
            std::cout << area << std::endl;
            if(area < 150 && area > 30)
            {
                std::cout<<"achou semafaro vermelho"<<std::endl;
                cv::drawContours(trafic_draw, contours, j, cv::Scalar(0, 255, 0), 1, cv::LINE_8);
            }
        }
    }
    else
    {
        std::cout <<"nao achou sinal vermelho"<<std::endl;
    }
    
    cv::imwrite("./found_traffic_ligths.bmp", trafic_draw);
    return 0;
}
