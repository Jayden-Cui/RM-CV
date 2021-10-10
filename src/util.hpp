#ifndef __UTIL_HPP__
#define __UTIL_HPP__


#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


struct Hsv
{
    int iLowH;
    int iHighH;

    int iLowS;
    int iHighS;

    int iLowV;
    int iHighV;
};

void controlBar(Hsv &range);

void hsvRange(Mat &input, Mat &output, Hsv &range);

void saveHsv(Hsv &range, string path);
void loadHsv(Hsv &range, string path);


void drawRotatedRect(Mat &img, RotatedRect &rect, Scalar color=Scalar(255,0,255), int thickness=1);


float distance(RotatedRect &a, RotatedRect &b);
float distance(Point2f &a, Point2f &b);
float distance(const float &a, const float &b);


float angle(Vec2f vec);
float angle(const Point2f &begin, const Point2f &end);

#endif