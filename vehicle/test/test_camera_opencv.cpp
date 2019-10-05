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
	//Open camera
	cout<<"Opening Camera..."<<endl;
	if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}
	//Start capture
	cout<<"Capturing "<<nCount<<" frames ...."<<endl;
	time ( &timer_begin );
	for ( int i=0; i<nCount; i++ ) {
		Camera.grab();
		Camera.retrieve ( image);
		if ( i%5==0 )  cout<<"\r captured "<<i<<" images"<<std::flush;
	}
	for ( int i=0; i<10; i++ ) {
		Camera.grab();
		Camera.retrieve (image);
		stringstream ss;
		ss << "raspicam_cv_image" << i << ".png";
		cv::imwrite(ss.str(), image);
		usleep(1000000);
	}
	cout<<"Stop camera..."<<endl;
	Camera.release();
	//show time statistics
	time ( &timer_end ); /* get current time; same as: timer = time(NULL)  */
	double secondsElapsed = difftime ( timer_end,timer_begin );
	cout<< secondsElapsed<<" seconds for "<< nCount<<"  frames : FPS = "<<  ( float ) ( ( float ) ( nCount ) /secondsElapsed ) <<endl;
	//save image 
	cv::imwrite("raspicam_cv_image.jpg",image);
	cout<<"Image saved at raspicam_cv_image.jpg"<<endl;
}