#include "energydetector.hpp"



EnergyDetector::EnergyDetector(/* args */)
{
    loadData();
}

EnergyDetector::~EnergyDetector()
{
    saveData();
}

void EnergyDetector::setControlBar()
{
}

void EnergyDetector::setInputImage(Mat img)
{
    img.copyTo(frame);

    if ( center_rect )   {
        delete center_rect;
        center_rect = nullptr;
    }
    if ( target_rect )   {
        delete target_rect;
        target_rect = nullptr;
    }
    armors.clear();
}

void EnergyDetector::preTreatment()
{
    if ( frame.empty() ) {
        cout << "=== EMPTY IMAGE ===";
        return; 
    }
    
    Mat hsv;
    cvtColor(frame, hsv, COLOR_RGB2HSV);

    Mat mask;
    hsvRange(hsv, mask, energyHsv);

    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(7, 7));    
    morphologyEx(mask, mask, MORPH_CLOSE, kernel1);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(mask, mask, MORPH_OPEN, kernel2);
    // imshow("mask", mask);


	vector<vector<Point>> contours;
	vector<Vec4i> hierachy;
    findContours(mask, contours, hierachy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE);

    int armor_cnt = 0;
    for (int i=0; i>=0; i=hierachy[i][0]) {
    // for (int i=0; i<hierachy.size(); i++) {
        float area = contourArea(contours[i]);
        if ( area < 100 )   continue;

        RotatedRect rect = minAreaRect(contours[i]);
        float ratio = max(rect.size.height, rect.size.width) \
            / min(rect.size.height, rect.size.width);
        if ( area < 300 && ratio > 1.5 )    continue;

        int cnt = 0, son_id = hierachy[i][2];
        while ( son_id != -1 ) {
            son_id = hierachy[son_id][0];
            cnt ++;
        }
        // putText(frame, to_string(cnt), rect.center, FONT_HERSHEY_COMPLEX, 1, Scalar(0,255,255));
        // 已击打过的灯条
        if ( cnt > 1 ) {
            drawRotatedRect(frame, rect);

            son_id = hierachy[i][2];
            while ( son_id != -1 ) {
                RotatedRect armor = minAreaRect(contours[son_id]);
                if ( isArmor(armor) )
                    armors.push_back(armor);
                son_id = hierachy[son_id][0];
            }
        }
        // 将要击打的灯条
        else if ( cnt == 1 ) {
            int armor_id = hierachy[i][2];
            RotatedRect armor = minAreaRect(contours[armor_id]);
            if ( ! isArmor(armor) ) break;

            // TODO:
            // 遇冲突时比较两个装甲板哪个更合适
            if ( target_rect )   {
                cout << "confusing target\t"; 
                delete target_rect;
            }
            target_rect = new RotatedRect(minAreaRect(contours[armor_id]));

            drawRotatedRect(frame, *target_rect, Scalar(240,55,55), 3);
            // putText(frame, to_string(target->size.width/target->size.height), target->center, FONT_HERSHEY_COMPLEX, 1, Scalar(255));
            circle(frame, target_rect->center, 8, Scalar(25,255,255), FILLED);

        }
        // 中心
        else {
            drawContours(frame, contours, i, Scalar(215,255,55), FILLED);
            if ( center_rect ) {
                cout << "confusing center\t";
                delete center_rect;
            }

            center_rect = new RotatedRect(rect);
            // circle(frame, center_rect->center, 3, Scalar(255,255,255), FILLED);
            
        }
        // drawContours(frame, contours, i, Scalar(255,25,255), 2);

        armor_cnt ++;
    }
    putText(frame, to_string(armor_cnt), Point(0, frame.size().height), FONT_HERSHEY_COMPLEX, 1, Scalar(255,255,255),3);

}


void EnergyDetector::calculate()
{
    if ( center_rect == nullptr ) {
        if ( last_status != CENTER_NOT_FOUND )
            cout << "center not found\t";
        last_status = CENTER_NOT_FOUND;
        return;
    }
    if ( target_rect == nullptr ) {
        if ( last_status != TARGET_NOT_FOUND )
            cout << "target not found\t";
        last_status = TARGET_NOT_FOUND;
        return;
    }

    last_status = FOUND;


    Point2f center_point = center_rect->center;
    Point2f target_point = target_rect->center;
    float target_angle = angle(center_point, target_point);


    line(frame, center_point, target_point, Scalar(0,25,255), 3);
    putText(frame, to_string(target_angle), target_point, FONT_HERSHEY_COMPLEX, 1, Scalar(0,190,255), 2);

    vector<float> distances;
    distances.push_back(distance(center_point, target_point));
    
    for (auto armor : armors) {
        line(frame, center_point, armor.center, Scalar(255,255,0), 1);
        distances.push_back(distance(center_point, armor.center));
        putText(frame, to_string(angle(center_point,armor.center)), armor.center, FONT_HERSHEY_COMPLEX, 1, Scalar(255,100,55), 2);

    }
    for (auto d : distances)
        cout << d << " ";

}

void EnergyDetector::showFrame()
{
    imshow("frame", frame);
}


bool EnergyDetector::isArmor(RotatedRect &rect)
{
    if ( rect.size.area() < 700 )   return false;
    float ratio = rect.size.width / rect.size.height;
    if ( ratio < 1 )    ratio = 1 / ratio;
    if ( ratio < 1.3 || ratio > 2 ) return false;

    return true;
}



void EnergyDetector::saveData()
{
    saveHsv(energyHsv, ENERGY_PATH);
}

void EnergyDetector::loadData()
{
    loadHsv(energyHsv, ENERGY_PATH);
}

