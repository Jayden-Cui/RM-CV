#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

#include "armordetection.hpp"
#include "energydetector.hpp"
#include "grayarmor.hpp"

enum DETECTION_MODE{
	ARMOR,
	ENERGY,
	GRAY_ARMOR
};

#define DATA1_IMAGE 0x01
#define DATA2_VIDEO 0x02
#define DATA3_GRAY_IMAGE 0x03


#define MODE GRAY_ARMOR
#define DATASET DATA3_GRAY_IMAGE
#define HIGH_LIGHT true

using namespace cv;
using namespace std;

Detector *gray_armor = new GrayArmor();
ArmorDetection* armor = new ArmorDetection();
EnergyDetector* energy = new EnergyDetector(HIGH_LIGHT);
EnergyDetector* energy_future = new EnergyDetector(HIGH_LIGHT, true);
bool control = false;


#define MAXPICNUM 1095


void parse(int argc, char **argv)
{
	for (int i=1; i<argc; i++) {
		if ( string(argv[i]) == "control" )
			control = true;
		else
			cout << "arg [" << string(argv[i]) << "] not found" << endl;  
	}
	if ( control ) {
		if ( MODE == GRAY_ARMOR )	gray_armor->control();
		else if ( MODE == ENERGY )  controlBar(energy->energyHsv);
	}
}

int main(int argc, char **argv)
{
	cout.flags(ios::fixed);
	cout.precision(2);
	parse(argc, argv);

	Mat frame;
	vector<Mat*> frames;
	Mat *PtrMat;
	

	cout << "============= Loading Video =============" << endl;
	// 任务： 灰灭装甲板的识别
#if (DATASET == DATA3_GRAY_IMAGE)
	vector<string> pic_names = {"1671", "2203", "6400", "6480", "21600"};
	for (auto name : pic_names) {
		PtrMat = new Mat;
		imread("../sources/gray_armor/"+name+".jpg").copyTo(*PtrMat);
		frames.push_back(PtrMat);
	}
#elif (DATASET == DATA2_VIDEO )
	// 培训2： 大能量机关
	string video_path;
	if ( HIGH_LIGHT )
		video_path = "../sources/video/highlight.mp4";
	else
		video_path = "../sources/video/video.mp4";
	VideoCapture cap(video_path);
	while ( cap.read(frame) ) {
		PtrMat = new Mat;
		frame.copyTo(*PtrMat);
		frames.push_back(PtrMat);
	}
#elif (DATASET == DATA1_IMAGE)
	// 培训1： 装甲板的识别数据集
	for (int i=0; i<MAXPICNUM; i++) {
		PtrMat = new Mat;
		std::string img_path = "../sources/imageDataset/" + to_string(i) + ".jpg";
		imread(img_path).copyTo(*PtrMat);
		frames.push_back(PtrMat);
	}
#endif
	cout << "Loaded: " << frames.size() << " frames in total" << endl;

	bool auto_play = false;
	bool show_pred = false;
	int gap = 200;
	int id = 0;
	int last_id = -1;

	for ( ; ; )
	{
		if ( last_id != id ) 
		{
			cout << "\r[" << id << "/" << frames.size() << "]" << " ";
			frames[id]->copyTo(frame);
			// putText(frame, to_string(id), Point(0, 30), HersheyFonts::FONT_HERSHEY_COMPLEX, 1, Scalar(0xff,0xff,0xf), 2);
			
			if ( MODE == ARMOR ) {
				armor->setInputImage(frame);
				armor->run();
			}
			else if ( MODE == ENERGY ) {
				energy->setInputImage(frame);
				energy->run();
				if ( show_pred ) {
					Mat future;
					if ( id+PRED_FRAME < frames.size() && id+PRED_FRAME>=0 )
						frames[id+PRED_FRAME]->copyTo(future);
					energy_future->setInputImage(future);
					energy_future->preTreatment();
					energy_future->drawFuturePoint(*energy);
				}
				energy->showFrame();
				energy->printResult();
				// energy->saveAngle();
			}
			else if ( MODE == GRAY_ARMOR ) {
				gray_armor->setInput(frame);
				gray_armor->run();
			}
			cout << endl;
		}

		last_id = id;
		char c = waitKey(gap);
		if (c == 27 || c == 'q' ) //"Esc"
			break;
		else if ( c == 'a' ) {
			id -= (id>0);
		}
		else if ( c == 'd' ) {
			id += (id<frames.size()-1);
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
		else if ( c == 'b' ) {
			show_pred = !show_pred;
		}
		else if ( auto_play ) {
			id += (id<frames.size()-1);
		}
	}

	if ( control ) {
		if ( MODE == ARMOR )			armor->saveData();
		else if ( MODE == ENERGY )		energy->saveData();
		else if ( MODE == GRAY_ARMOR )	gray_armor->saveData();
	}

	return 0;
}

