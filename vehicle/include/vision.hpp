#ifndef vision_h_
#define vision_h_

#include <vector>
#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>
#include "street_section.hpp"

class Vision
{
	private:
		float static const horizon_y = 0.282;
		float static const real_zero_deg = 4.647;
		float static const img_height = 960;
		float static const img_width = 1280;
		raspicam::RaspiCam_Cv cam;
		cv::Mat img;
		void getColorMask(cv::Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max);
		void getRedTapeMask(cv::Mat& dst);
		void getGreenTapeMask(cv::Mat& dst);
		void getBlueTapeMask(cv::Mat& dst);
		void getYellowTapeMask(cv::Mat& dst);
	public:
		void getCamImg();
		std::vector<StreetSection> findStreets();
		Vision();
};

#endif //vision_h_
