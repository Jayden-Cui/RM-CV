#ifndef __ENERGY_DETECTOR_HPP__
#define __ENERGY_DETECTOR_HPP__


#include <opencv2/opencv.hpp>
#include "util.hpp"


#define PRED_FRAME 5

using namespace std;
using namespace cv;

#define IF_THEN_RETURN(condiction, status) \
    if ( condiction ) { \
        last_status = status; \
        return; \
    }

class EnergyDetector
{
private:
    string ENERGY_PATH = "../model/energyhsv.dat";
    string HIGH_LIGHT_PATH = "../model/highlighthsv.dat";
    string ANGLE_PATH = "../model/angle.dat";

    bool high_light_mode;
    bool silence;
    Mat frame;

    vector<RotatedRect> armors;
    RotatedRect *center_rect = nullptr;
    RotatedRect *target_rect = nullptr;
    vector<Angle> angles;
    Point2f last_center;
    float radius;
    int  lost_frames;

    bool isArmor(RotatedRect &rect);
    bool likeArmor(const float father_area, vector<Point> &son);
    bool likeCenter(vector<Point> cent);

public:
    enum status {
        DONE,
        FOUND,
        IMAGE_EMPTY,
        CONTOUR_NOT_FOUND,
        CENTER_NOT_FOUND,
        TARGET_NOT_FOUND
    };
    status last_status;

    Hsv energyHsv;
    Hsv highLightHsv;

    EnergyDetector(bool hlm=false, bool slc=false);
    ~EnergyDetector();

    void setInputImage(Mat &img);
    void drawFuturePoint(EnergyDetector &detector);

    void preTreatment();
    void calculate();
    void showFrame();
    void predict();
    void printResult();


    void saveAngle();
    void saveData();
    void loadData();
};



#endif