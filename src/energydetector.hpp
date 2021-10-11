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
    string HIGH_LIGHT_PATH = "../model/highlighthsv.dat";
    string ANGLE_PATH = "../model/angle.dat";

    bool high_light_mode;
    Mat frame;

    vector<RotatedRect> armors;
    RotatedRect *center_rect = nullptr;
    RotatedRect *target_rect = nullptr;
    vector<float> angles;
    float last_angle;
    float radius;

    bool isArmor(RotatedRect &rect);
public:
    enum status {
        FOUND,
        CENTER_NOT_FOUND,
        TARGET_NOT_FOUND
    };
    status last_status;

    Hsv energyHsv;
    Hsv highLightHsv;

    EnergyDetector(bool hlm=false);
    ~EnergyDetector();

    void setInputImage(Mat img);
    void drawFuturePoint(EnergyDetector &detector);

    void preTreatment();
    void calculate();
    void showFrame();
    void predict();

    void saveAngle();
    void saveData();
    void loadData();
};



#endif