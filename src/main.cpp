#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#include "armordetection.h"
#include "energydetector.hpp"

enum DETECTION_MODE{
	ARMOR,
	ENERGY
};

#define MODE ENERGY
#define HIGH_LIGHT false

using namespace cv;
using namespace std;

ArmorDetection* armor = new ArmorDetection();
EnergyDetector* energy = new EnergyDetector(HIGH_LIGHT);
EnergyDetector* energy_future = new EnergyDetector(HIGH_LIGHT);
Point2f center;

#define MAXPICNUM 1095

void parse(int argc, char **argv)
{
	for (int i=1; i<argc; i++) {
		if ( string(argv[i]) == "control:main" )
			controlBar(armor->mainHsv);
		else if ( string(argv[i]) == "control:blue" )
			controlBar(armor->blueHsv);
		else if ( string(argv[i]) == "control:energy" ) {
			if ( HIGH_LIGHT )
				controlBar(energy->highLightHsv);
			else
				controlBar(energy->energyHsv);
		}
	}
}

int main(int argc, char **argv)
{
	parse(argc, argv);

	Mat frame;
	vector<Mat*> frames;
	Mat *PtrMat;
	

	string video_path;
	if ( HIGH_LIGHT )
		video_path = "../sources/video/video2.mp4";
	else
		video_path = "../sources/video/video.mp4";
	VideoCapture cap(video_path);

	cout << "============= Loading Video =============" << endl;
	while ( cap.read(frame) ) {
		PtrMat = new Mat;
		frame.copyTo(*PtrMat);
		frames.push_back(PtrMat);
	}
	cout << "Loaded: " << frames.size() << " frames in total" << endl;

	bool auto_play = false;
	int gap = 100;
	int id;
	int last_id = -1;

	for (id=0; ; )
	{
		cout << "\r[" << id << "/" << frames.size() << "]" << " ";
		if ( last_id != id ) 
		{
			frames[id]->copyTo(frame);
			// frame = frames[id];
			// std::string img_path = "../sources/imageDataset/" + to_string(id) + ".jpg";
			// frame = imread(img_path);
			// if ( frame.empty() ) {
			// 	id += (id > last_id ? 1 : -1);
			// 	continue;
			// }
			imshow("orginframe", frame);
			putText(frame, to_string(id), Point(0, 30), HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(0xff,0xff,0xff), 2);
			
		}
			if ( MODE == ARMOR ) {
				armor->setInputImage(frame);
				armor->Pretreatment();
				armor->getArmor();
				armor->showFrame();
			}
			else if ( MODE == ENERGY ) {
				energy->setInputImage(frame);
				energy->preTreatment();
				energy->calculate();
				energy->predict();

				// Mat future;
				// if ( id+5 < frames.size() )
				// 	frames[id+5]->copyTo(future);
				// energy_future->setInputImage(future);
				// energy_future->preTreatment();
				// energy_future->drawFuturePoint(*energy);

				energy->showFrame();
				// energy->saveAngle();
			}

			cout << endl;
		last_id = id;
		char c = waitKey(gap);
		if (c == 27 || c == 'q' ) //"Esc"
			break;
		else if ( c == 'a' ) {
			id -= (id>0);
		}
		else if ( c == 'd' ) {
			id += (id<frames.size());
		}
		else if ( c == 'z' ) {
			id -= (id>9) * 10;
		}
		else if ( c == 'c' ) {
			id += (id<frames.size()-10) * 10;
		}
		else if ( c == 's' ) {
			armor->saveData();
			energy->saveData();
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
			id += (id<frames.size()-1);
		}
	}

	armor->saveData();
	energy->saveData();
	return 0;
}

