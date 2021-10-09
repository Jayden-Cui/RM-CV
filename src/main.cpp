#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "armordetection.h"

using namespace cv;
using namespace std;

ArmorDetection* armor = new ArmorDetection();
Point2f center;

#define MAXPICNUM 1095

void parse(int argc, char **argv)
{
	for (int i=1; i<argc; i++) {
		if ( string(argv[i]) == "control:main" )
			armor->controlBar(armor->mainHsv);
		else if ( string(argv[i]) == "control:blue" )
			armor->controlBar(armor->blueHsv);
	}
}

int main(int argc, char **argv)
{
	parse(argc, argv);

	Mat frame;
	bool auto_play = false;

	int gap = 100;
	int last_id = -1;
	// while (capture.read(frame))//读取当前帧
	for (int id=0; ; )
	{
		// cout << "\r[" << id << "/" << 1000 << "]";
		if ( last_id != id ) {

			std::string img_path = "../../../imageDataset/" + to_string(id) + ".jpg";
			frame = imread(img_path);
			if ( frame.empty() ) {
				id += (id > last_id ? 1 : -1);
				continue;
			}
			imshow("orginframe", frame);
			putText(frame, to_string(id), Point(0, 30), HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(0xff,0xff,0xff), 2);
			armor->setInputImage(frame);
			armor->Pretreatment();
			armor->getArmor();
			armor->showFrame();

		}
		last_id = id;
		char c = waitKey(gap);
		if (c == 27 || c == 'q' ) //"Esc"
			break;
		else if ( c == 'a' ) {
			id -= (id>0);
		}
		else if ( c == 'd' ) {
			id += (id<MAXPICNUM);
		}
		else if ( c == 'z' ) {
			id -= (id>9) * 10;
		}
		else if ( c == 'c' ) {
			id += (id<MAXPICNUM-10) * 10;
		}
		else if ( c == 's' ) {
			armor->saveData();
		}
		else if ( c == 'j' ) {
			auto_play = true;
		}
		else if ( c == 'k' ) {
			auto_play = false;
		}
		else if ( c == 'u' ) {
			gap /= 2;
		}
		else if ( c == 'i' ) {
			gap *= 2;
		}
		else if ( auto_play ) {
			id += (id<MAXPICNUM);
		}
	}
	delete armor;
	waitKey(1);
	return 0;
}

