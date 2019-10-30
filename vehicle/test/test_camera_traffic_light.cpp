#include <cstdio>
#include <cmath>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


int main()
{
    const std::string path =  "./img/street/";
    const std::string img_in = "traffic_light";
    const std::string extension = ".png";

    auto img = cv::imread(path + img_in + extension);
    if (img.data == NULL)
    {
        printf("File not found");
        return -1;
    }
    cv::cvtColor(img, img, cv::COLOR_BGR2HLS);

    auto h_min = 160;
    auto h_max = 180;
    auto l_min = 100;
    auto l_max = 240;
    auto s_min = 40;
    auto s_max = 140;

    cv::Mat red_mask;
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), red_mask);
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
                std::cout<<"achou semafaro vermelho"<<std::endl;
            }
        }
    }
    else
    {
        std::cout <<"nao achou sinal vermelho"<<std::endl;
    }
    
    return 0;
}
