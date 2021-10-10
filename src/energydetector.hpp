#ifndef __ENERGY_DETECTOR_HPP__
#define __ENERGY_DETECTOR_HPP__


#include <opencv2/opencv.hpp>
#include "util.hpp"


using namespace std;
using namespace cv;

class EnergyDetector
{
private:
    string ENERGY_PATH = "../model/energyhsv.dat";
    Mat frame;
    vector<RotatedRect> armors;
    RotatedRect *center_rect;
    RotatedRect *target_rect;

    bool isArmor(RotatedRect &rect);
public:
    enum status {
        FOUND,
        CENTER_NOT_FOUND,
        TARGET_NOT_FOUND
    };
    status last_status;

    Hsv energyHsv;

    EnergyDetector(/* args */);
    ~EnergyDetector();

    void setControlBar();
    void setInputImage(Mat img);

    void preTreatment();
    void calculate();
    void showFrame();

    void saveData();
    void loadData();
};



#endif