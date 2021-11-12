#include "armordetection.hpp"
#include "lightboard.hpp"

ArmorDetection::ArmorDetection() {
	LoadData();
	center = Point2f(-1, -1);
};

ArmorDetection::ArmorDetection(Mat & input) {
	LoadData();
	frame = input;
}

void ArmorDetection::setInputImage(Mat input) {
	frame = input;
	width = frame.size().width;
	height = frame.size().height;

	lostRect = false;
	minRects.clear();

	LightBoard::set_clear(false);
	LightBoard::result = LightBoard::EMPTY;
}


//图像预处理
void ArmorDetection::Pretreatment() {
	clock_t start = clock();

	// namedWindow("control");
	// createTrackbar("minarea", "control", &param.minArea, 500);
	// createTrackbar("fillity", "control", &param.fillity, 100);
	// createTrackbar("ratio", "control", &param.ratio, 300);

	// get hsv image
	Mat hsv;
	blur(frame, frame, Size(5, 5));
	cvtColor(frame, hsv, CV_BGR2HSV);

	// main inrange
	Mat mask;
	hsvRange(hsv, mask, mainHsv);
	morphologyEx(mask, mask, MORPH_OPEN, kernel1);
	morphologyEx(mask, mask, MORPH_CLOSE, kernel1);
	dilate(mask, mask, kernel2);    //膨胀

	// blue inrange
	Mat blueMask;
	hsvRange(hsv, blueMask, blueHsv);
	morphologyEx(blueMask, blueMask, MORPH_OPEN, kernel1);
	morphologyEx(blueMask, blueMask, MORPH_CLOSE, kernel3);
	dilate(blueMask, blueMask, kernel1);

	mask = mask | blueMask;
	// imshow("mask", mask);

	// open morpholog again
	morphologyEx(mask, mask, MORPH_OPEN, kernel1);

	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	findContours(mask, contours, hireachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//筛选，去除一部分矩形
	int area_padoff = contours.size() < 20 ? 35 : 0;
	// double area_padoff = contours.size(
	for (int i = 0; i < contours.size(); ++i) {
		/*******  primary filter  ************/
		// points enough
		if ( contours[i].size() < 6 )	continue;

		// no children contours
		if ( hireachy[i][2] != -1 )	continue;
		
		// light area must be large enough
		double cont_area = contourArea(contours[i]);
		if ( cont_area < param.minArea - area_padoff )	continue;
		
		// fit ellipse
		RotatedRect minRect = fitEllipse(contours[i]);

		// ellipse(frame, minRect, Scalar(188,155,55), 1);

		if ( minRect.angle > 30 && minRect.angle < 150 )	continue;
		// ellipse(frame, minRect, Scalar(88,55,255), 1);

		// if ( cont_area / minRect.size.area() < 0.6 )	continue;

		double ratio = max(minRect.size.width, minRect.size.height) \
			/ min(minRect.size.width, minRect.size.height);
			

		// long short rate
		if ( ratio < double(param.ratio)/100. )
			continue;

		char str_ratio[8];
		sprintf(str_ratio, "%.2f", cont_area / minRect.size.area());
		// putText(frame, string(str_ratio), minRect.center, FONT_HERSHEY_COMPLEX, 0.6, Scalar(5,55,255), 2);

		// ellipse(frame, minRect, Scalar(255,0,255), 3);

		minRects.push_back(minRect);
	}

	putText(frame, to_string(minRects.size()), Point(0,frame.size().height-10), HersheyFonts::FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255,255,0));

	if ( ! minRects.empty() ) {
		sort(minRects.begin(), minRects.end(), \
			[](const RotatedRect&a, const RotatedRect&b) {
				return a.center.y < b.center.y;
		});
	}

}

void ArmorDetection::getArmor() {
	if ( minRects.empty() )	return;
	vector<vector<RotatedRect>> rect_groups;
	vector<RotatedRect> rect_group{minRects[0]};
	for (int i=1; i<=minRects.size(); i++) {
		if ( i == minRects.size() ) {
			if ( rect_group.size() > 1 )
				rect_groups.push_back(rect_group);
			break;
		}
		else if ( minRects[i].center.y - minRects[i-1].center.y > height / 15 ) {
			if ( rect_group.size() > 1 ) {
				sort(rect_group.begin(), rect_group.end(), \
					[](const RotatedRect &a, const RotatedRect &b) {
						return a.center.x < b.center.x;
					});
				rect_groups.push_back(rect_group);
			}
			rect_group.clear();
			rect_group.push_back(minRects[i]);
		}
		else {
			rect_group.push_back(minRects[i]);
		}
	}
	if ( rect_groups.empty() )	return;
	LightBoard *fitness = nullptr;

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
		fitness->drawArmor(frame);
		center = fitness->center();
		delete fitness;
	}

	if ( LightBoard::if_clear() ) {
		center = Point2f(-1, -1);
	}
}

void ArmorDetection::showFrame() {
	imshow("frame", frame);

	if ( LightBoard::EMPTY == LightBoard::result \
		|| LightBoard::LOST_TARGET == LightBoard::result ) {
		cout << "lost target" << endl;
	}	
	else if ( LightBoard::NEW_TARGET == LightBoard::result ) {
		cout << "perhaps new target" << endl;
	}
}

void ArmorDetection::run() {
	Pretreatment();
	getArmor();
	showFrame();
}

ArmorDetection::~ArmorDetection() {
}

void ArmorDetection::saveData() {
	saveHsv(mainHsv, MAIN_HSV_PATH);
	saveHsv(blueHsv, BLUE_HSV_PATH);
	saveHsv(redHsv, RED_HSV_PAHT);
}

void ArmorDetection::LoadData() {
	loadHsv(mainHsv, MAIN_HSV_PATH);
	loadHsv(blueHsv, BLUE_HSV_PATH);
	loadHsv(redHsv, RED_HSV_PAHT);
}