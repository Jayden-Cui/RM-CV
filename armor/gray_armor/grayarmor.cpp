#include "grayarmor.hpp"
#include "lightboard.hpp"


using namespace std;
using namespace cv;

GrayArmor::GrayArmor()
    : Detector("../model/gray_main.dat")
{
    loadData();
}

GrayArmor::~GrayArmor()
{
}

void GrayArmor::initRun()
{
    rect_possible_vec.clear();
    best_pair.clear();
}



void GrayArmor::preTreatment()
{
    Mat hsv;
    Mat kernel_3x3_ell = getStructuringElement(MORPH_ELLIPSE, Size(3,3));
    Mat kernel_3x3_ret = getStructuringElement(MORPH_RECT, Size(3,3));
    Mat kernel_5x5_ell = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
    Mat kernel_5x5_ret = getStructuringElement(MORPH_RECT, Size(5,5));
    Mat kernel_7x7_ell = getStructuringElement(MORPH_ELLIPSE, Size(7,7));

    // resize(src, src, Size(src.size().width/2, src.size().height/2));
    resize(src, src, Size(640, 512));
    src.copyTo(result);


    /**********  Main color detect  **********/
    cvtColor(src, hsv, CV_RGB2HSV);
    hsvRange(hsv, bin_main, main_hsv);
    morphologyEx(bin_main, bin_main, MORPH_CLOSE, kernel_3x3_ell);
    morphologyEx(bin_main, bin_main, MORPH_OPEN, kernel_3x3_ell);

    /**********  gray detect  **********/
    static int l=49, h=80;
    cvtColor(src, bin_gray, CV_BGR2GRAY);
    inRange(bin_gray, Scalar(l), Scalar(h), bin_gray);
    morphologyEx(bin_gray, bin_gray, MORPH_CLOSE, kernel_3x3_ell);
    // namedWindow("track");
    // createTrackbar("low", "track", &l, 255);
    // createTrackbar("high", "track", &h, 255);

    /**********  dark detect  **********/
    hsvRange(hsv, bin_dark, dark_hsv);
    bitwise_not(bin_dark, bin_dark);
    bitwise_and(bin_dark, bin_gray, bin_dark);


    bitwise_or(bin_dark, bin_main, bin);
    morphologyEx(bin, bin, MORPH_CLOSE, kernel_3x3_ret);
    morphologyEx(bin, bin, MORPH_CLOSE, kernel_3x3_ell);
    // morphologyEx(bin, bin, MORPH_OPEN, kernel_3x3_ret);
}


void GrayArmor::getContour()
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    vector<RotatedRect> rect_main;
    vector<RotatedRect> gray_rect_possible_vec;

    /**********  brightness detect  **********/
    do {
        findContours(bin_main, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        if ( contours.empty() ) break;
        for (int i=0; i<contours.size(); i++) {
            if ( contours[i].size() < 6 )   continue;

            RotatedRect rect = fitEllipse(contours[i]);
            rect_main.push_back(rect);
            // angle 
            if ( rect.angle > ANGLE_RANGE && 180-rect.angle > ANGLE_RANGE )
                continue;
            // ellipse(src, rect, Scalar(255,255,0), FILLED);

            rect_possible_vec.push_back(rect);
        }
        if ( rect_possible_vec.size() >= 2 )
            return;
    } while ( 0 );


    do {

        findContours(bin, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        for (int i=0; i<contours.size(); i++) {
            double raw_area = contourArea(contours[i]);
            if ( raw_area > 250 || raw_area < 3 \
                || contours[i].size() < 6 )
                continue;
            
            RotatedRect rect = fitEllipse(contours[i]);
            if ( rect_main.size() >= 1 ) {
                bool near_flag = false;
                for (int j=0; j<rect_main.size(); j++) {
                    if ( distance(rect_main[j], rect) < 100 ) {
                        near_flag = true;
                        continue;
                    }
                }
                if ( ! near_flag )  continue;
            }
            else {
                if ( large_div_small(rect.size.width,rect.size.height) < 2.5 )
                    continue;
            }
            rect_possible_vec.push_back(rect);
            // drawContours(src, contours, i, Scalar(255,0,0), FILLED);
        }
    } while ( 0 );
}

void GrayArmor::findBestPair()
{
    if ( rect_possible_vec.size() < 2 ) return;
    sort(rect_possible_vec.begin(), rect_possible_vec.end(), \
        [](const RotatedRect &a, const RotatedRect &b) {
            return a.center.y < b.center.y;
        });


    vector<vector<RotatedRect>> rect_groups;
    vector<RotatedRect> rect_group{rect_possible_vec[0]};
	for (int i=1; i<=rect_possible_vec.size(); i++) {
		if ( i == rect_possible_vec.size() ) {
			if ( rect_group.size() > 1 )
				rect_groups.push_back(rect_group);
			break;
		}
		else if ( rect_possible_vec[i].center.y-rect_possible_vec[i-1].center.y \
            > src.size().height / 15 ) {
			if ( rect_group.size() > 1 ) {
				sort(rect_group.begin(), rect_group.end(), \
					[](const RotatedRect &a, const RotatedRect &b) {
						return a.center.x < b.center.x;
					});
				rect_groups.push_back(rect_group);
			}
			rect_group.clear();
			rect_group.push_back(rect_possible_vec[i]);
		}
		else {
			rect_group.push_back(rect_possible_vec[i]);
		}
	}

    if ( rect_groups.empty() )  return;
	LightBoard *fitness = nullptr;
    Point2f center(-1, -1);

	for (int i=0; i<rect_groups.size(); i++) {
		for (int j=0; j<rect_groups[i].size()-1; j++) {
			// ellipse(frame, rect_groups[i][j], Scalar(255,0,255), 3);

			for (int k=j+1; k<rect_groups[i].size(); k++) {
				float this_score = LightBoard(rect_groups[i][j], rect_groups[i][k], center).score();
				if ( fitness == nullptr ) {
					if ( this_score > 0 )
						fitness = new LightBoard(rect_groups[i][j], rect_groups[i][k], center);
				}
				else if ( fitness->score() < LightBoard(rect_groups[i][j], rect_groups[i][k], center).score() ) {
					delete fitness;
					fitness = new LightBoard(rect_groups[i][j], rect_groups[i][k], center);
				}
			}
		}
	}
	if ( fitness != nullptr ) {
		fitness->drawArmor(result);
        delete fitness;
	}


}


void GrayArmor::run()
{
    initRun();

    preTreatment();
    getContour();
    findBestPair();

    imshow("src", src);
    imshow("result", result);
    // imwrite(img_name+".jpg", result);
}


void GrayArmor::control()
{
    controlBar(dark_hsv);
}


void GrayArmor::saveData()
{
    saveHsv(main_hsv, main_hsv_path);
    saveHsv(gray_hsv, GRAY_HSV_PATH);
    saveHsv(dark_hsv, DARY_HSV_PATH);
}
void GrayArmor::loadData()
{
    loadHsv(main_hsv, main_hsv_path);
    loadHsv(gray_hsv, GRAY_HSV_PATH);
    loadHsv(dark_hsv, DARY_HSV_PATH);
}

