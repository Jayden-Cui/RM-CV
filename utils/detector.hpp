#ifndef __DETECTOR_HPP__
#define __DETECTOR_HPP__

#include <opencv2/opencv.hpp>
#include "util.hpp"

class Detector
{
protected:
    Hsv main_hsv;
    cv::Mat src;
    std::string main_hsv_path;
    std::string img_name;
    static int img_id;
public:
    Detector( std::string path ) \
     : main_hsv_path(path)
    {
        loadData();
    }
    virtual ~Detector() { }

    virtual void run() = 0;
    virtual void setInput(cv::Mat &input, \
        std::string name="unkonwn_"+std::to_string(++img_id)) \
    {
        img_name = name;
        src.release();
        input.copyTo(src);
    }
    virtual void control()
    {
        controlBar(main_hsv);
    }

	virtual void saveData()
    {
        saveHsv(main_hsv, main_hsv_path);
    }
	virtual void loadData()
    {
        loadHsv(main_hsv, main_hsv_path);
    }

};




#endif