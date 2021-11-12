#pragma once
#ifndef ARMORDETECTION_H
#define ARMORDETECTION_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include "util.hpp"

#define hsvPath  "../model/hsv.dat"
#define MAIN_HSV_PATH	"../model/mainhsv.dat"
#define BLUE_HSV_PATH	"../model/bluehsv.dat"
#define RED_HSV_PAHT	"../model/redhsv.dat"

using namespace std;
using namespace cv;

class ArmorDetection {
private:
	Mat frame;
	Mat kernel1 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(-1, -1));
	Mat kernel2 = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(-1, -1));
	Mat kernel3 = getStructuringElement(MORPH_CROSS, Size(7, 7));
	vector<RotatedRect> minRects;
	int width, height;
	Point2f center;
	
	bool lostRect;
public:


	Hsv mainHsv;
	Hsv blueHsv;
	Hsv redHsv;
	
	ArmorDetection();
	explicit ArmorDetection(Mat& input);

	void setInputImage(Mat input);
	void Pretreatment();
	void getArmor();

	void showFrame();

	void run();

	~ArmorDetection();
	
	void saveData();
	void LoadData();

	struct
	{
		int minArea = 100;
		int fillity = 60;
		int ratio = 170;
	} param;
	

private:
};



#endif 

