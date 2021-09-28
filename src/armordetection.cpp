#include "armordetection.h"
#include <fstream>

#define hsvPath  "hsv.dat"

ArmorDetection::ArmorDetection() {
	LoadData();
};

ArmorDetection::ArmorDetection(Mat & input) {
	frame = input;
	LoadData();
}

void ArmorDetection::setInputImage(Mat input) {
	frame = input;
	currentCenter.x = 0;
	currentCenter.y = 0;
}

//图像预处理
void ArmorDetection::Pretreatment() {
	Mat canny;
	Point p, center;
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	vector<Rect> boundRect(contours.size());
	Point2f vertex[4];

	//创建进度条
	cvCreateTrackbar("LowH", "Control", &iLowH, 255);
	cvCreateTrackbar("HighH", "Control", &iHighH, 255);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255);
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255);
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	cvtColor(frame, hsv, CV_BGR2HSV);
	inRange(hsv,
		Scalar(iLowH, iLowS, iLowV),
		Scalar(iHighH, iHighS, iHighV),
		mask);
	// 形态学操作
	Canny(mask, canny, 100, 200);
	// morphologyEx(mask, mask, MORPH_OPEN, kernel1, Point(-1, -1));//开操作
	// dilate(mask, mask, kernel2, Point(-1, -1), 1);//膨胀

	//轮廓增强
	// Canny(mask, mask, 3, 9, 3);
	findContours(canny, contours, hireachy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//筛选，去除一部分矩形
	for (int i = 0; i < contours.size(); ++i) {
		RotatedRect minRect = minAreaRect(Mat(contours[i]));
		// minRect.points(vertex);
		if (minRect.size.width < minRect.size.height) {
			// cout << minRect.size.width << endl;
			// minRect.angle += 90;
			// float t = minRect.size.width;
			// minRect.size.width = minRect.size.height;
			// minRect.size.height = t;
			// rectangle(frame, contours[i], Scalar(255, 255, 0), 3);
			drawContours(frame, contours, i, Scalar(255,0,255), FILLED);
		}
		// rectangle(frame, contours[i], Scalar(255,255,0), 3);
	// 	if ((minRect.size.width * 10 > minRect.size.height)
	// 		&& (minRect.size.width * 1 < minRect.size.height)
	// 		&& (abs(minRect.angle) < 30)) {
	// 		minRects.push_back(minRect);
	// 	}
	// 	for (int l = 0; l < 4; l++)
	// 	{
	// 		line(frame, vertex[l], vertex[(l + 1) % 4], Scalar(255, 0, 0), 2);
	// 	}
	// 	line(frame, Point(frame.cols / 2 - 15, frame.rows / 2),
	// 		Point(frame.cols / 2 + 15, frame.rows / 2), Scalar(0, 255, 255), 5);
	// 	line(frame, Point(frame.cols / 2, frame.rows / 2 - 15),
	// 		Point(frame.cols / 2, frame.rows / 2 + 15), Scalar(0, 255, 255), 5);
	// 	circle(frame, Point(frame.cols / 2, frame.rows / 2), 4, Scalar(0, 0, 255), -1);
	}
	imshow("orgin", frame);
	imshow("maskask", mask);
	imshow("cont", canny);
}


Point2f ArmorDetection::GetArmorCenter() {
	//遍历所有矩形，两两组合
	RotatedRect leftRect, rightRect;
	vector<int*> reliability;
	double area[2], distance, height;

	if (minRects.size() < 2) {
		LostTarget();
		return currentCenter;
	}

	for (int i = 0; i < minRects.size(); ++i) {
		for (int j = i + 1; j < minRects.size(); ++j) {
			int level = 0;
			int temp[3];
			leftRect = minRects[i];
			rightRect = minRects[j];

			//判断
			if (leftRect.angle == rightRect.angle) {
				level += 10;
			}
			else if (abs(leftRect.angle - rightRect.angle) < 5) {
				level += 8;
			}
			else if (abs(leftRect.angle - rightRect.angle) < 10) {
				level += 6;
			}
			else if (abs(leftRect.angle - rightRect.angle) < 30) {
				level += 4;
			}
			else if (abs(leftRect.angle - rightRect.angle) < 90) {
				level += 1;
			}
			else {
				break;
			}

			area[0] = leftRect.size.width * leftRect.size.height;
			area[1] = rightRect.size.width * rightRect.size.height;
			if (area[0] == area[1]) {
				level += 10;
			}
			else if (min(area[0], area[1]) * 1.5 > max(area[0], area[1])) {
				level += 8;
			}
			else if (min(area[0], area[1]) * 2 > max(area[0], area[1])) {
				level += 6;
			}
			else if (min(area[0], area[1]) * 3 > max(area[0], area[1])) {
				level += 4;
			}
			else if (min(area[0], area[1]) * 4 > max(area[0], area[1])) {
				level += 1;
			}
			else {
				break;
			}

			double half_height = (leftRect.size.height + rightRect.size.height) / 4;
			if (leftRect.center.y == rightRect.center.y) {
				level += 10;
			}
			else if (abs(leftRect.center.y - rightRect.center.y) < 0.2 * half_height) {
				level += 8;
			}
			else if (abs(leftRect.center.y - rightRect.center.y) < 0.4 * half_height) {
				level += 6;
			}
			else if (abs(leftRect.center.y - rightRect.center.y) < 0.8 * half_height) {
				level += 4;
			}
			else if (abs(leftRect.center.y - rightRect.center.y) < half_height) {
				level += 1;
			}
			else {
				break;
			}

			distance = Distance(leftRect.center, rightRect.center);
			height = (leftRect.size.height + rightRect.size.height) / 2;
			if (distance != 0 && distance > height) {
				if (distance < 1.5 * height) {
					level += 6;
				}
				else if (distance < 1.8 * height) {
					level += 4;
				}
				else if (distance < 2.4 * height) {
					level += 2;
				}
				else if (distance < 10 * height) {
					level += 1;
				}
				else {
					break;
				}
			}

			temp[0] = i;
			temp[1] = j;
			temp[2] = level;

			reliability.push_back(temp);

		}
	}

	if (reliability.empty()) {
		LostTarget();
		return currentCenter;
	}
	else {

		int maxLevel = 0, index = 0;
		for (int k = 0; k < reliability.size(); ++k) {
			if (reliability[k][2] > maxLevel) {
				maxLevel = reliability[k][2];
				index = k;
			}
		}

		currentCenter.x = (minRects[reliability[index][0]].center.x + minRects[reliability[index][1]].center.x) / 2;
		currentCenter.y = (minRects[reliability[index][0]].center.y + minRects[reliability[index][1]].center.y) / 2;

		//与上一次的结果对比
		if (lastCenter.x == 0 && lastCenter.y == 0) {
			lastCenter = currentCenter;
			lost = 0;
		}
		else {
			double difference = Distance(currentCenter, lastCenter);
			if (difference > 300) {
				LostTarget();
				return currentCenter;
			}
		}
		line(frame, Point(currentCenter.x - 10, currentCenter.y - 10),
			Point(currentCenter.x + 10, currentCenter.y + 10), Scalar(255, 255, 0), 5);
		line(frame, Point(currentCenter.x + 10, currentCenter.y - 10),
			Point(currentCenter.x - 10, currentCenter.y + 10), Scalar(255, 255, 0), 5);
		circle(frame, currentCenter, 7.5, Scalar(0, 0, 255), 5);
		// imshow("frame", frame);
		return currentCenter;
	}
}

void ArmorDetection::LostTarget() {
	lost++;
	if (lost < 3) {
		currentCenter = lastCenter;
	}
	else {
		currentCenter = Point2f(0, 0);
		lastCenter = Point2f(0, 0);
	}
}

double ArmorDetection::Distance(Point2f a, Point2f b) {
	return sqrt((a.x - b.x) * (a.x - b.x) +
		(a.y - b.y) * (a.y - b.y));
}

double ArmorDetection::max(double first, double second) {
	return first > second ? first : second;
}

double ArmorDetection::min(double first, double second) {
	return first < second ? first : second;
}

ArmorDetection::~ArmorDetection() {
	saveData();
}

void ArmorDetection::saveData() {
	cout << "Saving" << endl;
	ofstream outFile;
	// cout << hsvPath << endl;
	
	outFile.open(hsvPath);	
	// outFile.write((char*)(&iLowH), 4);
	// outFile.write((char*)(&iHighH), 4);
	// outFile.write((char*)(&iLowS), 4);
	// outFile.write((char*)(&iHighS), 4);
	// outFile.write((char*)(&iLowV), 4);
	// outFile.write((char*)(&iHighV), 4);
	// cout << iLowH << endl;
	outFile << iLowH << " " << iHighH << " " << iLowS << " " << iHighS << " " << iLowV << " " << iHighV ;
	// cout << iLowH << iHighH << endl;
	outFile.close();
	cout << "Saved" << endl;
}

void ArmorDetection::LoadData() {
	cout << "Loading ..." << endl;
	ifstream inFile;
	// cout << hsvPath << endl;
	inFile.open(hsvPath, ios::in);
	// if ( inFile ) {
	// 	cout << "File not exist. Creating new file" << endl;
	// 	saveData(); 
	// }
	// inFile.read((char*)(&iLowH), 4);
	// inFile.read((char*)(&iHighH), 4);
	// inFile.read((char*)(&iLowS), 4);
	// inFile.read((char*)(&iHighS), 4);
	// inFile.read((char*)(&iLowV), 4);
	// inFile.read((char*)(&iHighV), 4);
	if ( !inFile.is_open()) {
		cout << "Open Fail" << endl;
		return ;
	}
	inFile >> iLowH >> iHighH >> iLowS >> iHighS >> iLowV;
	// cout << iLowH << endl;
	inFile.close();

	cout << "Loaded" << endl;
}