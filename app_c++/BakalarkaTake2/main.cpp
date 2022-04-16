// BakalarkaTake2.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <QFile>
#include <QMainWindow>
#include <QApplication>
#include <opencv2/opencv.hpp>
#include "widgets/MainWindow/MainWindow.h"

#include <QVBoxLayout>


#define NONE 99999999
int px = NONE;
int py;
Superellipse el2;

#define H 500
#define W 500

void mouseCallback(int event, int x, int y, int flags, void* userdata) {
	if (event == cv::EVENT_LBUTTONDOWN) {
		px = x;
		py = y;
	}
	if (event == cv::EVENT_LBUTTONUP) {
		qDebug((QString::number(x) + ", " + QString::number(y)).toStdString().c_str());
		px = NONE;
	}
	if (event == cv::EVENT_MOUSEMOVE) {
		if (px != NONE) {
			el2.setH((-x + H / 2));
			el2.setK(y);
		}
	}
}

int main(int argc, char* argv[])
{/*
	Superellipse el1 = Superellipse(100, 50, 2.5, 10, 150);
	el2 = Superellipse(150, 150, 2.5, 0, 0);
	cv::namedWindow("img");
	cv::setMouseCallback("img", mouseCallback);

	//auto pts = el1.borderPoints();
	//for (const auto& pt1 : pts) {
	//	cv::Mat img = cv::Mat::zeros(H, W, CV_8U);
	//	el1.draw(img);
	//	auto pt = cv::Point(lround(pt1.x() + W / 2), lround(-pt1.y()));
	//	pt.x = std::clamp(pt.x, 0, W);
	//	pt.y = std::clamp(pt.y, 0, H);
	//	cv::circle(img, pt, 5, 255);

	//	cv::imshow("img", img);
	//	auto key = cv::waitKey(10);
	//	if (key == 27) { // esc
	//		break;
	//	}
	//}

	while (true) {
		cv::Mat img = cv::Mat::zeros(H, W, CV_8U);
		el1.draw(img);
		el2.draw(img);
		auto pts = el1.findPOIs(el2);
		for (auto& pt : pts) {
			auto npt = cv::Point(lround(pt.x() + W / 2), lround(-pt.y()));
			npt.x = std::clamp(npt.x, 0, W);
			npt.y = std::clamp(npt.y, 0, H);
			cv::circle(img, npt, 5, 255);
		}

		//cv::line(img, cv::Point(-el1.h()+W/2, el1.k() + el1.b()), cv::Point(-el2.h()+W / 2, el2.k() + el2.b()), 255);

		cv::imshow("img", img);
		auto key = cv::waitKey(10);
		if (key == 27) { // esc
			break;
		}
		/*
			img = np.zeros((H, W), np.uint8)
		el1.draw(img)
		el2.draw(img)
		pts = el1.findPOI(el2)
		# img = cv.rectangle(img, pt1, pt2, 100, 1)

		for pt in pts:
			pt = (round(pt[0]+W//2), round(-pt[1]))
			pt = clamp(pt)
			img = cv.circle(img, pt, 5, 255)
		cv.imshow("img", img)
		key = cv.waitKey(10)
		if key == 27:  # esc
			break


	cv::destroyAllWindows();
	*/
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
	auto app = QApplication(argc, argv);
	auto window = MainWindow();

	QFile file(":/gui/style/style.qss");
	file.open(QFile::ReadOnly);
	if (file.isOpen()) {
		QString styleSheet = QString(file.readAll());
		app.setStyleSheet(styleSheet);
	}

	window.resize(800, 800);
	window.show();
	return app.exec();
}
