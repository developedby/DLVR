#ifndef vision_hpp_
#define vision_hpp_

#include <vector>
#include <tuple>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/videoio.hpp>
#include "streets.hpp"
#include "ultrasound.hpp"

class Vision
{
	private:
		raspicam::RaspiCam_Cv downward_cam;
		cv::VideoCapture forward_cam;
		cv::Ptr<cv::aruco::Dictionary> city_aruco_dict;
		cv::Ptr<cv::aruco::Dictionary> app_aruco_dict;
		Ultrasound ultrasound;
		cv::Mat getColorMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max);
		std::pair<std::vector<int>, std::vector<std::vector<cv::Point2f>>> findARMarkers(cv::Ptr<cv::aruco::Dictionary>);
	public:
		cv::Mat downward_img;
		cv::Mat forward_img;
		Vision();
		void getDownwardCamImg();
		void getForwardCamImg();
		std::pair<std::vector<streets::StreetSection>, std::vector<streets::StreetSection>> findStreets();
		bool isTrafficLightRed();
		std::pair<std::vector<int>, std::vector<std::vector<cv::Point2f>>> findCityARMarkers();
		std::pair<std::vector<int>, std::vector<std::vector<cv::Point2f>>> findAppARMarkers();
		float distanceFromObstacle();
};

#endif //vision_h_
