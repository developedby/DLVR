#ifndef vision_h_
#define vision_h_

#include <raspicam/raspicam_cv.h>
#include <opencv2/core.hpp>

class Vision
{
	private:
		raspicam::RaspiCam_Cv cam;
		cv::Mat getCamImg();
		cv::Mat img;
	public:
		void getColorMask(cv::Mat dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max);
		void getRedTapeMask(cv::Mat dst);
		void getGreenTapeMask(cv::Mat dst);
		void getBlueTapeMask(cv::Mat dst);
		void getYellowTapeMask(cv::Mat dst);
		Vision();
};

#endif //vision_h_
