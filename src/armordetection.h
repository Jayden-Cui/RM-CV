#pragma once
#ifndef ARMORDETECTION_H
#define ARMORDETECTION_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ArmorDetection {
private:
	Mat frame, hsv, mask;
	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	Mat kernel2 = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
	Point2f currentCenter;
	Point2f lastCenter;
	vector<RotatedRect> minRects;
	int lost;
	// static const string hsvPath;

public:
	ArmorDetection();
	explicit ArmorDetection(Mat& input);
	void setInputImage(Mat input);
	void Pretreatment();
	Point2f GetArmorCenter();
	~ArmorDetection();
	
	int iLowH = 49;
	int iHighH = 93;

	int iLowS = 5;
	int iHighS = 95;

	int iLowV = 255;
	int iHighV = 255;

private:
	void LostTarget();
	double Distance(Point2f, Point2f);
	double max(double, double);
	double min(double, double);
	void saveData();
	void LoadData();
};



#endif 

