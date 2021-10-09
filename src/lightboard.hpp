#ifndef __LIGHT_BOARD_HPP__
#define __LIGHT_BOARD_HPP__


#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class LightBoard
{
private:
    RotatedRect left;
    RotatedRect right;
    Point2f &last_point;
    Point2f _center;
    float _score;
    static bool clear_flag;
    /* data */
public:
    enum result_code{
        EMPTY,
        SUCCESS,
        LOST_TARGET,
        NEW_TARGET
    };
    static result_code result;
    LightBoard(RotatedRect &l, RotatedRect &r, Point2f &lp);
    ~LightBoard();

    void drawArmor(Mat &frame);
    double score() const;
    Point2f center() const;

    static void set_clear(bool clr = true);
    static bool if_clear();
};

float distance(RotatedRect &a, RotatedRect &b);
float distance(Point2f &a, Point2f &b);
float distance(const float &a, const float &b);

#endif