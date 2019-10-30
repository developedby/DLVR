#include <cstdio>
#include <cmath>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


int main()
{
    const std::string path =  "./img/street/";
    const std::string img_in = "cruzamento";
    const std::string extension = ".png";

    auto img = cv::imread(path + img_in + extension);
    if (img.data == NULL)
    {
        printf("File not found");
        return -1;
    }
    cv::cvtColor(img, img, cv::COLOR_BGR2HLS);

    auto h_min = 100;
    auto h_max = 120;
    auto l_min = 70;
    auto l_max = 200;
    auto s_min = 30;
    auto s_max = 170;

    cv::Mat mask;
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), mask);
    auto krn = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, krn);

    std::vector<cv::Vec4i> lines;
    auto e1 = cv::getTickCount();
    cv::HoughLinesP(mask, lines, 1, 5*M_PI/180, 100, 20, 20);
    auto e2 = cv::getTickCount();
    printf("Tempo: %f\n", (e2 - e1)/cv::getTickFrequency());
    
    return 0;
}
