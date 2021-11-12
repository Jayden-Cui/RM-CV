#include "util.hpp"
#include <iostream>


using namespace std;
using namespace cv;


void controlBar(Hsv &range) {
	//创建进度条
	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	cvCreateTrackbar("LowH", "Control", &range.iLowH, 179);
	cvCreateTrackbar("HighH", "Control", &range.iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &range.iLowS, 255);
	cvCreateTrackbar("HighS", "Control", &range.iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &range.iLowV, 255);
	cvCreateTrackbar("HighV", "Control", &range.iHighV, 255);

}


void hsvRange(Mat &input, Mat &output, Hsv &range) {
	inRange(input, 
		Scalar(range.iLowH, range.iLowS, range.iLowV),
		Scalar(range.iHighH, range.iHighS, range.iHighV),
		output);
}


void saveHsv(Hsv &range, string path) {
	ofstream out_file(path);
	out_file << range.iLowH << " " << range.iHighH << " " \
		<< range.iLowS << " " << range.iHighS << " " \
		<< range.iLowV << " " << range.iHighV;
	out_file.close();

	cout << "saveed [" << path << "]" << endl; 
}


void loadHsv(Hsv &range, string path) {
	ifstream in_file(path);
	if ( in_file.is_open() == false ) {
		cout << "Open " << path << " FAIL =============" << endl;
		return;
	}
	in_file >> range.iLowH >> range.iHighH \
		>> range.iLowS >> range.iHighS \
		>> range.iLowV >> range.iHighV;
	in_file.close();

	cout << "loaded [" << path << "]" << endl; 
}


void drawRotatedRect(Mat &img, RotatedRect &rect, Scalar color, int thickness)
{
	Point2f *pts = new Point2f[4];
	rect.points(pts);
	for (int i=0; i<4; i++) {
		line(img, pts[i], pts[(i+1)%4], color, thickness);
	}
}



float distance(RotatedRect &a, RotatedRect &b)
{
    return sqrt(pow(a.center.x - b.center.x, 2) + \
        pow(a.center.y - b.center.y, 2));
}

float distance(Point2f &a, Point2f &b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

float distance(const float &a, const float &b)
{
    return abs(a-b);
}

float angle(const Vec2f &vec)
{
	// 90 | 270
	if ( vec[0] == 0 )
		return vec[1] > 0 ? 90 : 270;
	float res = atan(vec[1]/vec[0]) * 180 / CV_PI;
	if ( vec[0] < 0 )
		res += 180;
	else if ( vec[1] < 0 )
		res += 360;
	return res ;
}

float angle(const Point2f &begin, const Point2f &end)
{
	Vec2f vec(end.x-begin.x, begin.y-end.y);
	return angle(vec);
}


float angle_gap(const float a1, const float a2)
{
	float gap = abs(a1-a2);
	return gap<180 ? gap : 360-gap;
}

double large_div_small(const double &a, const double &b)
{
    return max(a,b) / min(a,b);
}