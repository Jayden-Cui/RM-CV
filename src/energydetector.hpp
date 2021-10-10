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
    string ANGLE_PATH = "../model/angle.dat";

    Mat frame;

    vector<RotatedRect> armors;
    RotatedRect *center_rect;
    RotatedRect *target_rect;

    bool isArmor(RotatedRect &rect);

    float last_angle;
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

    void setInputImage(Mat img);
    void drawFuturePoint(EnergyDetector &detector);

    void preTreatment();
    void calculate();
    void showFrame();

    void saveAngle();
    void saveData();
    void loadData();
};



#endif