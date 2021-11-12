#ifndef __GRAY_ARMOR_HPP__
#define __GRAY_ARMOR_HPP__

#include "detector.hpp"
#include "util.hpp"

#define GRAY_HSV_PATH "../model/gray_gray.dat"
#define DARY_HSV_PATH "../model/gray_dark.dat"

#define ANGLE_RANGE 25

class GrayArmor : public Detector
{
private:
    Hsv gray_hsv;
    Hsv dark_hsv;
    std::vector<cv::RotatedRect> rect_possible_vec;
    std::vector<cv::RotatedRect> best_pair;
    cv::Mat bin_main, bin_gray, bin_dark, bin, result;

public:
    GrayArmor();
    ~GrayArmor();

    void initRun();

    void preTreatment();
    void getContour();
    void findBestPair();
    
	void saveData();
	void loadData();

    void control();

    void run();
};




#endif