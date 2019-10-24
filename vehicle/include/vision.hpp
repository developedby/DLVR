#ifndef vision_hpp_
#define vision_hpp_

#include <vector>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>
#include "street_section.hpp"

class Vision
{
	private:
		raspicam::RaspiCam_Cv cam;
		cv::Mat img;
		void getColorMask(cv::Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max);
		void getRedTapeMask(cv::Mat& dst);
		void getGreenTapeMask(cv::Mat& dst);
		void getBlueTapeMask(cv::Mat& dst);
		void getYellowTapeMask(cv::Mat& dst);
		void getWhiteTapeMask(cv::Mat& dst);
		void getGroundMask(cv::Mat& dst);
	public:
		void getCamImg();
		std::vector<street_lines::StreetSection> findStreets();
		Vision();
		bool isTrafficLightRed();
};

#endif //vision_h_
