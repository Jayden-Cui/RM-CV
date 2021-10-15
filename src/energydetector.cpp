#include "energydetector.hpp"


EnergyDetector::EnergyDetector(bool hlm)
{
    high_light_mode = hlm;
   
    loadData();

    // last_angle = 0;
    lost_frames = 0;
    last_status = DONE;

    ofstream out;
    out.open(ANGLE_PATH);
    out.close();
}

EnergyDetector::~EnergyDetector()
{
}

void EnergyDetector::setInputImage(Mat &img)
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

    if ( last_status == DONE ) {
        lost_frames = 0;
    }
    else {
        lost_frames ++;
    }
    if ( lost_frames > 7 ) {
        lost_frames = 0;
        cout << "lost ";
        angles.clear();
    }
}


void EnergyDetector::drawFuturePoint(EnergyDetector &detector)
{
    if (center_rect && target_rect)
    {
        if ( detector.center_rect ) {
            line(detector.frame, \
                detector.center_rect->center, \
                target_rect->center - center_rect->center + detector.center_rect->center, \
                Scalar(155, 155, 155), \
                2);
        }
        else
            line(detector.frame, center_rect->center, target_rect->center, Scalar(255,0,0), 2);
    }

}

void EnergyDetector::preTreatment()
{
    IF_THEN_RETURN(frame.empty(), IMAGE_EMPTY);

    /***************************
     * convert to 2 value image
    ***************************/
    blur(frame, frame, Size(3,3));

    Mat hsv;
    cvtColor(frame, hsv, COLOR_RGB2HSV);

    Mat mask;
    if ( high_light_mode )
        hsvRange(hsv, mask, highLightHsv);
    else
        hsvRange(hsv, mask, energyHsv);
    /* morphology operation : close -> open -> close*/
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(5, 5));    
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(mask, mask, MORPH_CLOSE, kernel1);
    morphologyEx(mask, mask, MORPH_OPEN, kernel2);
    morphologyEx(mask, mask, MORPH_CLOSE, kernel2);
    imshow("mask", mask);

    /***************************
     * find contours and match
    ***************************/
	vector<vector<Point>> contours;
	vector<Vec4i> hierachy;
    findContours(mask, contours, hierachy, CV_RETR_TREE, CHAIN_APPROX_SIMPLE);
    IF_THEN_RETURN(contours.empty(), CONTOUR_NOT_FOUND);
    /* match the armors and the center */ 
    int armor_cnt = 0;
    for (int i=0; i>=0; i=hierachy[i][0]) {
        float area = contourArea(contours[i]);
        if ( area < 100 )   continue;

        RotatedRect rect = minAreaRect(contours[i]);
        float ratio = max(rect.size.height, rect.size.width) \
            / min(rect.size.height, rect.size.width);

        // 检查内部轮廓数
        int cnt = 0, son_id = hierachy[i][2];
        for (; son_id != -1; son_id=hierachy[son_id][0]) {
            // filter some wrong area
            float son_area = contourArea(contours[son_id]);
            if ( son_area < 20 || son_area/area > 0.6)
                continue;
            else if ( likeArmor(area, contours[son_id]) )
                cnt ++;
        }
        // putText(frame, to_string(cnt), rect.center, FONT_HERSHEY_COMPLEX, 1, Scalar(0,255,255));
        // 已击打过的灯条
        if ( cnt > 1 && cnt < 4 ) {
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
            // if ( max(rect.size.height, rect.size.width) \
            //     / min(rect.size.height, rect.size.width) < 1.3 )
            //     continue;
            int armor_id = hierachy[i][2];
            RotatedRect armor = minAreaRect(contours[armor_id]);
            if ( ! isArmor(armor) ) continue;

            // TODO:
            // 遇冲突时比较两个装甲板哪个更合适
            if ( target_rect )   {
                cout << "confusing target\t"; 
                delete target_rect;
            }
            target_rect = new RotatedRect(minAreaRect(contours[armor_id]));

            drawRotatedRect(frame, *target_rect, Scalar(240,55,55), 3);
            circle(frame, target_rect->center, 8, Scalar(25,255,255), FILLED);
        }
        // 中心
        else if ( cnt == 0 ) {
            if ( !likeCenter(contours[i]) ) continue;
            drawContours(frame, contours, i, Scalar(55,255,55), FILLED);
            if ( center_rect ) {
                cout << "confusing center\t";
                delete center_rect;
            }

            center_rect = new RotatedRect(rect);
        }

        armor_cnt ++;
    }
    cout << "armors:" << armor_cnt << " ";
}


void EnergyDetector::calculate()
{
    IF_THEN_RETURN(center_rect==nullptr, CENTER_NOT_FOUND);
    IF_THEN_RETURN(target_rect==nullptr, TARGET_NOT_FOUND);

    last_status = FOUND;

    Point2f &center_point = center_rect->center;
    Point2f &target_point = target_rect->center;
    Angle target_angle = angle(center_point, target_point);
    // Angle similar_angle = target_angle;

    radius = distance(center_point, target_point);

    vector<float> distances{radius};
    RotatedRect special_armor = *target_rect;
    for (auto armor : armors) {
        Angle armor_angle = angle(center_point,armor.center);
        line(frame, center_point, armor.center, Scalar(255,255,0), 1);
        distances.push_back(distance(center_point, armor.center));
        // putText(frame, to_string(armor_angle), armor.center, FONT_HERSHEY_COMPLEX, 1, Scalar(255,100,55), 2);
        // if ( abs(similar_angle-last_angle) > abs(armor_angle-last_angle) ) {
        //     similar_angle = armor_angle;
        //     special_armor = armor;
        // }
    }
    // last_angle = similar_angle;
    drawRotatedRect(frame, special_armor, Scalar(0,255,0), 4);

    // circle(frame, center_point, static_cast<int>(distance(center_point, target_point)+target_rect->size.width/2), Scalar(255,55,0), 1);

    if ( angles.empty() ) {
        angles.push_back(target_angle);
    }
    else {
        if ( angles.size() > 6 ) {
            angles.erase(angles.begin());
        }
        if ( abs(angles.back() - target_angle) > 30 ) {
            angles.clear();
            cout << "clear " ;
        }
        angles.push_back(target_angle);
    }

}

void EnergyDetector::predict()
{
    IF_THEN_RETURN(center_rect==nullptr, CENTER_NOT_FOUND);
    if ( angles.size() > 4 ) {
        vector<float> omegas;
        // vector<float> a;
        float a, omega = 0;
        
        Angle pred_angle;
        for (int i=1; i<angles.size(); i++) {
            omegas.push_back(angles[i]-angles[i-1]);
        }

        // a = omegas[omegas.size()-2] - omegas[omegas.size()-1];
        for (auto o : omegas)   omega += o;
        omega /= omegas.size();
        pred_angle = angles.back();
        for (int i=0; i<5; i++) {
            // omega += a;
            pred_angle = pred_angle + omega;
        }

        Point2f pred_point( \
            center_rect->center.x + radius * cos(pred_angle.rad()), \
            center_rect->center.y - radius * sin(pred_angle.rad()));
        circle(frame, pred_point, 8, Scalar(255,55,0), FILLED);
        line(frame, center_rect->center, pred_point, Scalar(255,55,0), 2);

        // for (int i=1; i<omegas.size(); i++) {
        //     a.push_back(angle_gap(omegas[i], omegas[i-1]));
        // } 

        last_status = DONE;
    }

}

void EnergyDetector::showFrame()
{
    imshow("frame", frame);
}

void EnergyDetector::printResult()
{
    switch ( last_status )
    {
    case IMAGE_EMPTY:
        cout << "Image is Empty " ;
        break;
    case CONTOUR_NOT_FOUND:
        cout << "Contour is Not Found " ;
        break;
    case CENTER_NOT_FOUND:
        cout << "Center is Not Found " ;
        break;
    case TARGET_NOT_FOUND:
        cout << "Target is Not Found " ;
        break;
    case DONE:
        cout << "DONE " ;
    default:
        cout << "nothing" ;
        break;
    }
}



bool EnergyDetector::isArmor(RotatedRect &rect)
{
    if ( rect.size.area() < 550 )   return false;
    float ratio = rect.size.width / rect.size.height;
    if ( ratio < 1 )    ratio = 1 / ratio;
    if ( ratio < 1.3 || ratio > 2.4 ) return false;

    return true;
}


bool EnergyDetector::likeArmor(const float father_area, vector<Point> &son)
{
    float son_area = contourArea(son);

    RotatedRect rect = minAreaRect(son);
    float ratio = rect.size.height / rect.size.width;
    if ( ratio < 1 ) ratio = 1/ratio;
    if ( ratio < 1.4 || ratio > 5 )
        return false;
    return true;
}

bool EnergyDetector::likeCenter(vector<Point> cent)
{
    RotatedRect rect = minAreaRect(cent);
    if ( rect.size.area() > 1500 ) return false;

    float ratio = rect.size.height / rect.size.width;
    if ( ratio < 1 )    ratio = 1/ratio;
    if ( ratio > 1.5 )
        return false;
    return true;
}



void EnergyDetector::saveAngle()
{
    ofstream out;
    out.open(ANGLE_PATH, ios::app);
    // out << last_angle.degree << endl;
}


void EnergyDetector::saveData()
{
    saveHsv(energyHsv, ENERGY_PATH);
    saveHsv(highLightHsv, HIGH_LIGHT_PATH);
}

void EnergyDetector::loadData()
{
    loadHsv(energyHsv, ENERGY_PATH);
    loadHsv(highLightHsv, HIGH_LIGHT_PATH);
}

