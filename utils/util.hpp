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

struct Angle
{
    float degree;
    Angle(){};
    Angle(float d): degree(d) {}

    float rad() {
        return degree / 180 * M_PI;
    }
    Angle operator +(Angle &b) {
        Angle res(degree + b.degree);
        if ( res.degree >= 360 )    res.degree -= 360;
        return res;
    }
    Angle operator + (const float &b) {
        Angle res(degree + b);
        if ( res.degree >= 360 )    res.degree -= 360;
        return res;
    }
    float operator -(Angle &b) {
        float res = degree - b.degree;
        if ( res > 180 )        res -= 360;
        else if ( res < -180 )  res += 360;
        return res;
    }
    float operator -(const float &b) {
        float res = abs(degree - b) ;
        if ( res > 180 )        res -= 360;
        else if ( res < -180 )  res += 360;
        return res;
    }

};

void controlBar(Hsv &range);

void hsvRange(Mat &input, Mat &output, Hsv &range);

void saveHsv(Hsv &range, string path);
void loadHsv(Hsv &range, string path);


void drawRotatedRect(Mat &img, RotatedRect &rect, Scalar color=Scalar(255,0,255), int thickness=1);


float distance(RotatedRect &a, RotatedRect &b);
float distance(Point2f &a, Point2f &b);
float distance(const float &a, const float &b);


float angle(const Vec2f &vec);
float angle(const Point2f &begin, const Point2f &end);

float angle_gap(const float a1, const float a2);

#endif