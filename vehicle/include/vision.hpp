#ifndef vision_hpp_
#define vision_hpp_

#include <vector>
#include <tuple>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/videoio.hpp>
#include "streets.hpp"

class Vision
{
	private:
		raspicam::RaspiCam_Cv downward_cam;
		cv::VideoCapture forward_cam;
		cv::Ptr<cv::aruco::Dictionary> aruco_dict;
		cv::Mat getColorMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max);
	public:
		cv::Mat downward_img;
		cv::Mat forward_img;
		Vision();
		void getDownwardCamImg();
		void getForwardCamImg();
		std::pair<std::vector<streets::StreetSection>, std::vector<streets::StreetSection>> findStreets();
		bool isTrafficLightRed();
		std::pair<std::vector<int>, std::vector<std::vector<cv::Point2f>>> findARMarkers();
};

#endif //vision_h_
