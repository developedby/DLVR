#include <ctime>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <raspicam/raspicam_cv.h>
using namespace std; 
 
int main ( int argc,char **argv ) {
   
	time_t timer_begin,timer_end;
	raspicam::RaspiCam_Cv Camera;
	cv::Mat image;
	int nCount=100;
	//set camera params
	Camera.set( cv::CAP_PROP_FORMAT, CV_8UC3 );
	Camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	Camera.set(cv::CAP_PROP_FRAME_HEIGHT, 960);
	//Camera.set(cv::CAP_PROP_SATURATION, 70);
	//Camera.setAWB(9);
	Camera.setRotation(3);
	//Open camera
	cout<<"Opening Camera..."<<endl;
	if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}
	//Start capture
	usleep(2000000);
	time ( &timer_begin );
	for ( int i=0; i<5; i++ ) {
		Camera.grab();
		Camera.retrieve (image);
		stringstream ss;
		ss << "raspicam_cv_image" << i << ".png";
		cv::imwrite(ss.str(), image);
		cout << "Tirando foto " << i << std::endl;
		//usleep(1000000);
	}
	cout<<"Stop camera..."<<endl;
	Camera.release();
	//show time statistics
	time ( &timer_end ); /* get current time; same as: timer = time(NULL)  */
	double secondsElapsed = difftime ( timer_end,timer_begin );
	cout<< secondsElapsed<<" seconds for "<< nCount<<"  frames : FPS = "<<  ( float ) ( ( float ) ( nCount ) /secondsElapsed ) <<endl;
}
