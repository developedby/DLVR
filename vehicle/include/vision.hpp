#ifndef vision_hpp_
#define vision_hpp_

#include <vector>
#include <tuple>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include "street_section.hpp"

class Vision
{
	private:
		raspicam::RaspiCam_Cv cam;
		cv::Ptr<cv::aruco::Dictionary> aruco_dict;
		void getColorMask(cv::Mat& dst, const cv::Scalar min, const cv::Scalar max);
		void getTapeMask(cv::Mat& dst, const cv::Scalar min, const cv::Scalar max);
		void getRedTapeMask(cv::Mat& dst);
		void getGreenTapeMask(cv::Mat& dst);
		void getBlueTapeMask(cv::Mat& dst);
		void getYellowTapeMask(cv::Mat& dst);
		void getWhiteTapeMask(cv::Mat& dst);
		void getGroundMask(cv::Mat& dst);
	public:
		cv::Mat img;
		Vision();
		void getCamImg();
		std::vector<street_lines::StreetSection> findStreets();
		bool isTrafficLightRed();
		std::pair<std::vector<int>, std::vector<std::vector<cv::Point2f>>> findARMarkers();
};

#endif //vision_h_
