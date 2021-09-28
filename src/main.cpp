#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "armordetection.h"

using namespace cv;
using namespace std;

ArmorDetection* armor = new ArmorDetection();
Point2f center;

int main(int argc, char **argv)
{
	//fps变量
	double t = (double)getTickCount();
	double fps;
	char string[10];
	char string2[10];
	Mat frame;


	// VideoCapture capture(argv[1]);
	// if (!capture.isOpened())
	// {
	// 	printf("无法打开相机...\n");
	// 	return -1;
	// }

	namedWindow("frame", CV_WINDOW_AUTOSIZE);
	namedWindow("mask", CV_WINDOW_AUTOSIZE);
	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	// while (capture.read(frame))//读取当前帧
	for (int id=0; id<1000;)
	{
		cout << "\r[" << id << "/" << 1000 << "]";
		std::string img_path = "../../../imageDataset/" + to_string(id) + ".jpg";
		frame = imread(img_path);
		armor->setInputImage(frame);
		armor->Pretreatment();
		// center = armor->GetArmorCenter();
		// cout << "[INFO] x = " << center.x - frame.cols / 2 << "    y = " << center.y - frame.rows / 2 << endl;

		//计算fps
		// double dt = ((double)getTickCount() - t) / (double)getTickFrequency();
		// fps = 1.0 / dt;
		// t = (double)getTickCount();
		// sprintf(string, "%.2f", fps);
		// std::string fpsString("FPS:");
		// fpsString += string;
		// putText(frame, fpsString, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255));

		char c = waitKey(100);
		if (c == 27) //"Esc"
		{
			break;
		}
		else if ( c == 'a' ) {
			id -= (id>0);
		}
		else if ( c == 'd' ) {
			id ++;
		}
	}
	delete armor;
	// capture.release();//释放视频内存
	waitKey(1);
	return 0;
}

