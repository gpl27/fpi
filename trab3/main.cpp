#include <opencv2/opencv.hpp>
using namespace cv;
int main(int argc, char** argv)
{
int camera = 0;
VideoCapture cap;
// open the default camera, use something different from 0 otherwise;
// Check VideoCapture documentation.
if(!cap.open(camera))
return 0;
for(;;)
{
Mat frame;
cap >> frame;
if( frame.empty() ) break; // end of video stream
imshow("This is you, smile! :)", frame);
if( waitKey(1) == 27 ) break; // stop capturing by pressing ESC
}
cap.release(); // release the VideoCapture object
return 0;
}