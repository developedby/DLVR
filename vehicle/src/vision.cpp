#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <tuple>
#include <vector>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/videoio.hpp>
#include "constants.hpp"
#include "streets.hpp"
#include "geometry.hpp"

using std::vector;
using std::pair;
using std::cout;
using std::endl;
using streets::StreetSection;

Vision::Vision() :
    downward_cam(), forward_cam(),
    city_aruco_dict(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250)),
    app_aruco_dict(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000)),
    downward_img(), forward_img()
{
    this->downward_cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    //this->downward_cam.setRotation(3);
    this->downward_cam.set(cv::CAP_PROP_FRAME_WIDTH, consts::img_width);
    this->downward_cam.set(cv::CAP_PROP_FRAME_HEIGHT, consts::img_height);
    this->downward_cam.setAWB(10);
    const auto success_cam1 = this->downward_cam.open();
    assert(success_cam1);
    for (int i = 0; i < 10; i++)
    {
        this->downward_cam.grab();
    }

    const auto success_cam2 = this->forward_cam.open(0);
    assert(success_cam2);
    //this->forward_cam.set(cv::CAP_PROP_FRAME_WIDTH, consts::img_width);
    //this->forward_cam.set(cv::CAP_PROP_FRAME_HEIGHT, consts::img_height);    
}

void Vision::getDownwardCamImg()
{
    this->downward_cam.grab();
    this->downward_cam.retrieve(this->downward_img);
    cv::blur(this->downward_img, this->downward_img, cv::Size(5, 5));
    cv::cvtColor(this->downward_img, this->downward_img, cv::COLOR_BGR2HLS);
}

void Vision::getForwardCamImg()
{
    //cv::Mat first_img;
    //this->forward_cam.read(first_img);
    this->forward_cam.read(this->forward_img);
    //this->forward_img += first_img;
   // this->forward_img /= 2;
}

// Looks at the image and finds the streets
// Returns a graph of the StreetSections identified
pair<vector<StreetSection>, vector<StreetSection>> Vision::findStreets()
{
    //cout << "Procurando linhas" << endl;
    
    // Finds the lines that go through the tapes in the image
    const auto [lines, lines_colors] = streets::findTapeLines(this->downward_img);
    /*cout << "Calculou as linhas. Encontradas: " << lines.size() << endl;
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        cout << lines[i] << ' ' << int(lines_colors[i]) << endl;
    }*/
    
    // Reverts the projection distortion, converting to StreetSection
    const vector<StreetSection> tape_sections = streets::undoProjectionDistortion(lines, lines_colors);
    //cout << "Reverteu a distorcao de perspectiva" << endl;
    //std::for_each(tape_sections.begin(), tape_sections.end(), [](const auto& sec){sec.print();});

    // Find all the possible street sections
    const vector<StreetSection> possible_sections = streets::findPossibleStreetSections(tape_sections);
    if (consts::save_img)
    {
        auto possible_secs_img = streets::drawStreetsAndTapes(tape_sections, possible_sections, true, true);
        cv::imwrite("teste_linhas_possible_secs.jpg", possible_secs_img);
    }
    //cout << "Calculou todas as mini seções. Encontradas: " << possible_sections.size() << endl;
    //std::for_each(possible_sections.begin(), possible_sections.end(), [](const auto& sec){sec.print();});

    // Transforms overlapping sections into a single long section
    const vector<StreetSection> long_sections = streets::groupIntoLongSections(possible_sections);
    if (consts::save_img)
    {
        auto long_secs_img = streets::drawStreetsAndTapes(tape_sections, long_sections, true, true);
        cv::imwrite("teste_linhas_long_secs.jpg", long_secs_img);
    }
    //cout << "Calculou todas as seções longas. Encontradas: " << long_sections.size() << endl;
    //std::for_each(long_sections.begin(), long_sections.end(), [](const auto& sec){sec.print();});

    // Break the sections where they intersect
    vector<StreetSection> final_sections = streets::breakIntersectingSections(long_sections);
    //cout << "Calculou as seções finais. Encontradas: " << final_sections.size() << endl;
    //std::for_each(final_sections.begin(), final_sections.end(), [](const auto& sec){sec.print();});
    
    // Link the sections
    buildSectionGraph(final_sections);
    //cout << "Juntou as seções" << endl;

    return pair(tape_sections, final_sections);
}

cv::Mat Vision::getColorMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max)
{
    // TODO: Remover a copia dessa versão em streets. Passar as duas pra outro lugar
    cv::Mat out;
    cv::inRange(img, min, max, out);
    const cv::Mat krn_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(out, out, cv::MORPH_OPEN, krn_open);
    const cv::Mat krn_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(out, out, cv::MORPH_CLOSE, krn_close);
    return out;
}

bool Vision::isTrafficLightRed()
{
    cv::Mat img;
    cv::GaussianBlur(this->forward_img, img, cv::Size(0,0), 3);
    cv::cvtColor(img, img, cv::COLOR_BGR2HLS);
    const cv::Rect traffic_light_roi(300, 0, 340, 300);
    const cv::Mat traffic_light_img = img(traffic_light_roi);
    cv::Mat red_mask;
    if(not consts::save_img)
    {
        cv::inRange(traffic_light_img, cv::Scalar(0, 100, 130), cv::Scalar(30, 230, 255), red_mask);
    }
    else
    {
        cv::inRange(img, cv::Scalar(0, 100, 130), cv::Scalar(30, 230, 255), red_mask);
    }
    cv::Mat green_mask;
    cv::inRange(traffic_light_img, cv::Scalar(70, 160, 90), cv::Scalar(100, 225, 255), green_mask);
    
    if (consts::save_img)
    {
        cv::imwrite("./borrado.jpg", img);
        cv::imwrite("./found_traffic_ligths_roi.jpg", traffic_light_img);
        cv::imwrite("./red_mask.jpg", red_mask);
        cv::imwrite("./gren_mask.jpg", green_mask);
    }
    
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    float biggest_possible_red_area = 0;
    cv::findContours(red_mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    int j = 0;
    for (const auto& contour: contours)
    {
        std::cout << "area: " << cv::contourArea(contour) << std::endl;
        float area = cv::contourArea(contour);
        if(consts::save_img and (area <= consts::max_traffic_light_area) and (area >= consts::min_traffic_light_area))
        {
            std::cout << "achou semafaro" << std::endl;
            cv::drawContours(this->forward_img, contours, j, cv::Scalar(0, 255, 0), 1, cv::LINE_8);
        }
        if((area <= consts::max_traffic_light_area) and (area >= consts::min_traffic_light_area) and (area > biggest_possible_red_area))
            biggest_possible_red_area = area;
        j++;
    }
    float biggest_possible_green_area = 0;
    cv::findContours(green_mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour: contours)
    {
        //std::cout << "area: " << cv::contourArea(contour) << std::endl;
        float area = cv::contourArea(contour);
        if((area <= consts::max_traffic_light_area) and (area >= consts::min_traffic_light_area) and (area > biggest_possible_red_area))
            biggest_possible_green_area = area; 
    }
    return (biggest_possible_red_area > biggest_possible_green_area);
    
}

pair<vector<int>, vector<vector<cv::Point2f>>> Vision::findCityARMarkers()
{
    cv::Mat second_img;
    this->forward_cam.read(second_img);
    second_img += this->forward_img;
    second_img /= 2;
    vector<int> ids;
    vector<vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(second_img, this->city_aruco_dict, corners, ids);
    return pair(ids, corners);
}

pair<vector<int>, vector<vector<cv::Point2f>>> Vision::findAppARMarkers()
{
    cv::Mat second_img;
    this->forward_cam.read(second_img);
    second_img += this->forward_img;
    second_img /= 2;
    vector<int> ids;
    vector<vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(second_img, this->app_aruco_dict, corners, ids);
    return pair(ids, corners);
}

pair<vector<int>, vector<vector<cv::Point2f>>> Vision::findARMarkers(cv::Ptr<cv::aruco::Dictionary> dict)
{
    vector<int> ids;
    vector<vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(this->forward_img, dict, corners, ids);
    return pair(ids, corners);
}

float Vision::distanceFromObstacle()
{
    return ultrasound.getDistance();
}
    
