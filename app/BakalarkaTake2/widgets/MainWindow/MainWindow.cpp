/*****************************************************************//**
 * \file   MainWindow.cpp
 * \brief  Implementation of the class
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <iostream>
#include <fstream>
#include <future>
#include <chrono>
#include <thread>
#include <charconv>


using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), m_cfg()
{
	setupUi(this);

	// initialize splitter segment sizes
	auto sizes = splitter->sizes();
	sizes[0] = 500;
	sizes[1] = 100;
	splitter->setSizes(sizes);

	connect(actionLoad_BSE_images, &QAction::triggered, this, &MainWindow::onLoadBSEImages);
	connect(actionShow_normal_image, &QAction::triggered, this, &MainWindow::onShowNormalImage);
	connect(actionPort_angle, &QAction::triggered, this, &MainWindow::onDetectorSettingsInvoked);
	connect(actionUse_default_material, &QAction::triggered, this, &MainWindow::onUseDefaultMaterial);
	connect(imageManager->selector(), &ImageLabel::selected, this, &MainWindow::onSelected);
	connect(imageManager->segmentManager(), &SegmentManager::swapped, this, &MainWindow::onSwapped);
}

MainWindow::~MainWindow()
{
	if (m_angleDialog) {
		delete m_angleDialog;
	}
}

void MainWindow::loadBSEImages(std::array<QString, 4> paths)
{
	if (paths[0] == "") {
		auto list = QFileDialog::getOpenFileNames(this,"Select 4 BSE images", "", "Images (*.png *.tif *.tiff)");
		if (list.size() != 4) {
			QMessageBox::critical(this, "Unable to open", "Please select 4 images.");
			return;
		}
		for (int i = 0; i < 4; i++) {
			paths[i] = list[i];
		}
	}
	double wd;
	double bc;
	double be;
	for (int i = 0; i < 4; i++) {
		int place = i;
		auto img = imread(paths[i].toStdString());
		if (img.empty()) {
			QMessageBox::critical(this, "Unable to open", "Image \"" + paths[i] + "\" could not be loaded.");
			return;
		}

		// look for .hdr file and load contents
		auto file = ifstream((paths[i].replace(QChar('.'), "-") + ".hdr").toStdString());
		std::string content;
		if (file.is_open()) {
			while (getline(file, content)) {
				if (content.starts_with("ImageStripSize=")) {
					try {
						int stripSize = stoi(content.substr(15));
						img = img(cv::Range(0, img.rows - stripSize), cv::Range(0, img.cols));
					}
					catch (std::exception& e) {
						std::cout << e.what() << "\n";
					}
				}
				else if (content.starts_with("BeamCurrent=")) {
					from_chars(content.c_str() + 12, content.c_str() + content.size(), bc);
				}
				else if (content.starts_with("AcceleratorVoltage=")) {
					from_chars(content.c_str() + 19, content.c_str() + content.size(), be);
				}
				else if (content.starts_with("WD=")) {
					from_chars(content.c_str() + 3, content.c_str() + content.size(), wd);
				}
				else if (content.starts_with("Detector=BSE Q")) {
					from_chars(content.c_str() + 14, content.c_str() + content.size(), place);
				}
			}
		}
		else {
			QMessageBox::critical(this, "Missing hdr file", "Image \"" + paths[i] + "\" does not have .hdr file associated with it.");
			return;
		}
		place--;
		if (place < 0 || place > 3)
		{
			place = i;
		}

		// downscale if too big
		int m = max(img.cols, img.rows);
		if (m > MAX_RES) {
			double coeff = MAX_RES / (double)m;
			cv::resize(img, img, cv::Size(img.cols * coeff, img.rows * coeff));
		}

		m_origImgs[place] = img;
	}

	// reset estimator with current values
	try {
		m_estimator.reset(be / 1000, wd * 1000, bc * 1e9);
	}
	catch (std::exception& e) {
		QMessageBox::critical(this, "Error", e.what());
		return;
	}

	processBSEImages();
}

void MainWindow::processBSEImages()
{
	for (int i = 0; i < 4; i++) {
		//rotate images
		m_origImgs[i].copyTo(m_imgs[i]);

		// apply brightness contrast
		cv::convertScaleAbs(m_imgs[i], m_imgs[i], m_cfg.alpha(i), m_cfg.beta(i));
	}

	// prepare grayscale images
	for (int i = 0; i < 4; i++) {
		cvtColor(m_imgs[i], m_grayImgs[i], COLOR_BGR2GRAY);
	}

	imageManager->loadImages(m_imgs);
}

void MainWindow::showNormalImage()
{
	auto filename = QFileDialog::getSaveFileName(this, "Normal file location", "", tr("TXT (*.txt)"));
	if (filename == "") {
		return;
	}

	QProgressDialog progress("Precalculating intersections..", "Abort", 0, 256 * 6, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);


	unsigned long status = 0;
	bool cancelPoint = false;
	std::vector<std::pair<Segments, Segments>> segs = { {Segments::Q1, Segments::Q2},
														{Segments::Q1, Segments::Q3},
														{Segments::Q1, Segments::Q4},
														{Segments::Q2, Segments::Q3},
														{Segments::Q2, Segments::Q4},
														{Segments::Q3, Segments::Q4},
	};
	// use 6 threads, one for each pair
	std::array<std::future<std::unique_ptr<std::array < std::array < std::vector<QPointF>, 256>, 256>>>, 6> futures;
	for (int i = 0; i < 6; ++i)
		futures[i] = std::async(std::launch::async, [&segs, i, this, &status, &cancelPoint]() { return precalculateIntersections(segs[i].first, segs[i].second, status, cancelPoint); });

	bool end = false;
	while (!end) {
		progress.setValue(status);
		end = true;
		for (int i = 0; i < 6; i++) {
			if (futures[i].wait_for(2ms) != std::future_status::ready) {
				end = false;
				break;
			}
		}
		if (progress.wasCanceled()) {
			cancelPoint = true;
		}
	}
	if (cancelPoint == true) {
		return;
	}
	progress.cancel();

	auto pts12 = futures[0].get();
	auto pts13 = futures[1].get();
	auto pts14 = futures[2].get();
	auto pts23 = futures[3].get();
	auto pts24 = futures[4].get();
	auto pts34 = futures[5].get();


	QProgressDialog progress2("Calculating normals...", "Abort", 0, m_grayImgs[0].cols, this);
	progress2.setWindowModality(Qt::WindowModal);

	auto file = std::ofstream(filename.toStdString());
	file << m_grayImgs[0].cols << " " << m_grayImgs[0].rows << "\n";

	cv::Mat finImg = cv::Mat(m_grayImgs[0].size(), CV_8UC3);
	for (int y = 0; y < m_grayImgs[0].rows; y++) {
		for (int x = 0; x < m_grayImgs[0].cols; x++) {
			int v1 = m_grayImgs[0].at<uint8_t>(y, x);
			int v2 = m_grayImgs[1].at<uint8_t>(y, x);
			int v3 = m_grayImgs[2].at<uint8_t>(y, x);
			int v4 = m_grayImgs[3].at<uint8_t>(y, x);

			auto res = evaluatePoints(pts12->at(v1)[v2], pts13->at(v1)[v3], pts14->at(v1)[v4], pts23->at(v2)[v3], pts24->at(v2)[v4], pts34->at(v3)[v4]);
			res.ry() += 0.5;
			res.ry() *= 2;
			res.rx() *= 2;
			double mid = pow(res.x(), 2) + pow(res.y(), 2);
			if (mid > 1) {
				double mag = sqrt(pow(res.x(), 2) + pow(res.y(), 2));
				res.rx() /= mag;
				res.ry() /= mag;
				mid = 0.999; // avoid 90 degrees inclination
			}
			double z = sqrt(1 - mid);
			// the reconstruction algorithm expects one coordinate negative for some reason
			file << -res.x() << " " << res.y() << " " << z << "\n";
			int r = lround(res.x() * 128 + 127);
			int g = lround(res.y() * 128 + 127);
			int b = lround(z * 128 + 127);
			finImg.at<Vec3b>(Point(x, y)) = Vec3b(b, g, r);
		}

		progress2.setValue(y);
		if (progress2.wasCanceled())
			return;
	}
	progress2.cancel();
	file.close();

	cv::imwrite("final.png", finImg);
	cv::imshow("Normal map", finImg);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

void MainWindow::onDetectorSettingsChanged()
{
	m_cfg.save();
	processBSEImages();
}

void MainWindow::onUseDefaultMaterial()
{
	// general reflectance maps
	m_estimator.reset(5, 10, 7);
}


void MainWindow::onSelected(double x, double y)
{
	reflectanceMap->reset();
	int trueX = m_imgs[0].cols * x;
	int trueY = m_imgs[0].rows * y;
	cout << trueX << ", " << trueY << endl;
	int a = m_grayImgs[0].at<uint8_t>(trueY, trueX);
	int b = m_grayImgs[1].at<uint8_t>(trueY, trueX);
	int c = m_grayImgs[2].at<uint8_t>(trueY, trueX);
	int d = m_grayImgs[3].at<uint8_t>(trueY, trueX);

	auto el1 = Superellipse(m_estimator.getA(a), m_estimator.getB(a), m_estimator.getC(a), 0, m_estimator.getK(a));
	reflectanceMap->drawSuperellipse(el1, Segments::Q1);
	auto el2 = Superellipse(m_estimator.getA(b), m_estimator.getB(b), m_estimator.getC(b), 0, m_estimator.getK(b));
	el2.changeSegment(Segments::Q2);
	reflectanceMap->drawSuperellipse(el2, Segments::Q2);
	auto el3 = Superellipse(m_estimator.getA(c), m_estimator.getB(c), m_estimator.getC(c), 0, m_estimator.getK(c));
	el3.changeSegment(Segments::Q3);
	reflectanceMap->drawSuperellipse(el3, Segments::Q3);
	auto el4 = Superellipse(m_estimator.getA(d), m_estimator.getB(d), m_estimator.getC(d), 0, m_estimator.getK(d));
	el4.changeSegment(Segments::Q4);
	reflectanceMap->drawSuperellipse(el4, Segments::Q4);

	auto npts12 = el1.findPOIs(el2);
	auto npts13 = el1.findPOIs(el3);
	auto npts14 = el1.findPOIs(el4);
	auto npts23 = el2.findPOIs(el3);
	auto npts24 = el2.findPOIs(el4);
	auto npts34 = el3.findPOIs(el4);

	evaluatePointsGraphic(npts12, npts13, npts14, npts23, npts24, npts34);
}

void MainWindow::onSwapped(int first, int second)
{
	swap(m_imgs[first], m_imgs[second]);
	swap(m_origImgs[first], m_origImgs[second]);
	swap(m_grayImgs[first], m_grayImgs[second]);
	processBSEImages();
}

void MainWindow::onDetectorSettingsInvoked(bool checked)
{
	if (!m_angleDialog) {
		m_angleDialog = new DetectorSettingsDialog(m_cfg, this);
		connect(m_angleDialog, &DetectorSettingsDialog::settingsUpdated, this, &MainWindow::onDetectorSettingsChanged);
	}
	m_angleDialog->update();
	m_angleDialog->show();
	m_angleDialog->raise();
	m_angleDialog->activateWindow();
}


QPointF MainWindow::evaluatePointsGraphic(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34)
{
	std::vector<QPointF> pts;
	pts.insert(pts.end(), npts12.begin(), npts12.end());
	pts.insert(pts.end(), npts13.begin(), npts13.end());
	pts.insert(pts.end(), npts14.begin(), npts14.end());
	pts.insert(pts.end(), npts23.begin(), npts23.end());
	pts.insert(pts.end(), npts24.begin(), npts24.end());
	pts.insert(pts.end(), npts34.begin(), npts34.end());


	for (const auto& pt : pts) {
		reflectanceMap->point(pt.x() + 0.5, -pt.y());
	}

	std::array<std::vector<QPointF>*, 6> grouped = { &npts12, &npts13, &npts14, &npts23, &npts24, &npts34 };
	std::array<std::vector<QPointF*>, 6> groupedPtr{};

	// create arrays of pointers to the points. If source array is empty, insert nullptr to allow deeper iterations
	int i = 0;
	for (const auto& group : grouped) {
		vector<QPointF*> tmp{ group->size(),nullptr };
		if (group->size() == 0) {
			tmp.push_back(nullptr);
		}
		else {
			std::transform(group->begin(), group->end(), tmp.begin(), [](auto& item) {return &item; });
		}
		groupedPtr[i] = tmp;
		i++;
	}

	double minDeviation = 99999999999999999999.;
	int minGroup = 0;
	const QPointF* curr[6]{};
	const QPointF* bestPoints[6]{};

	// terrible but minimum of memory copying
	for (const auto& g0 : groupedPtr[0]) {
		for (const auto& g1 : groupedPtr[1]) {
			for (const auto& g2 : groupedPtr[2]) {
				for (const auto& g3 : groupedPtr[3]) {
					for (const auto& g4 : groupedPtr[4]) {
						for (const auto& g5 : groupedPtr[5]) {
							curr[0] = g0;
							curr[1] = g1;
							curr[2] = g2;
							curr[3] = g3;
							curr[4] = g4;
							curr[5] = g5;


							// calculate average
							double sumptx = 0.;
							double sumpty = 0.;
							int n = 0;
							for (int point = 0; point < 6; point++) {
								if (!curr[point])
									continue;
								sumptx += curr[point]->x();
								sumpty += curr[point]->y();
								n++;
							}
							sumptx /= n;
							sumpty /= n;

							double deviation = 0.;
							// calculate deviation
							for (int point = 0; point < 6; point++) {
								if (!curr[point])
									continue;
								double x = curr[point]->x();
								double y = curr[point]->y();
								deviation += (sumptx - x) * (sumptx - x) + (sumpty - y) * (sumpty - y);
							}

							// determine if this is the best group so far
							if (deviation < minDeviation) {
								minDeviation = deviation;
								memcpy(bestPoints, curr, 6 * sizeof(QPointF*));
							}
						}
					}
				}
			}
		}
	}



	double sumptx = 0.;
	double sumpty = 0.;
	int n = 0;
	for (int point = 0; point < 6; point++) {
		if (!bestPoints[point])
			continue;
		double x = bestPoints[point]->x();
		double y = bestPoints[point]->y();
		sumptx += x;
		sumpty += y;
		reflectanceMap->point(x + 0.5, -y, Qt::cyan);
		n++;
	}

	if (n != 0)
	{
		sumptx /= n;
		sumpty /= n;
	}

	reflectanceMap->point(sumptx + 0.5, -sumpty, Qt::white);


	// rotate point
	double rangle = -m_cfg.portAngle() * (CV_PI / 180);
	double s = std::sin(rangle);
	double c = std::cos(rangle);
	double ty = sumpty + 0.5;
	double cx = sumptx * c - ty * s;
	double cy = sumptx * s + ty * c;
	cy -= 0.5;
	reflectanceMap->point(cx + 0.5, -cy, Qt::darkGreen);

	return QPointF(cx, cy);
}


QPointF MainWindow::evaluatePoints(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34)
{

	std::array<std::vector<QPointF>*, 6> grouped = { &npts12, &npts13, &npts14, &npts23, &npts24, &npts34 };
	std::array<std::vector<QPointF*>, 6> groupedPtr{};
	int i = 0;
	for (const auto& group : grouped) {
		vector<QPointF*> tmp{ group->size(),nullptr };
		if (group->size() == 0) {
			tmp.push_back(nullptr);
		}
		else {
			std::transform(group->begin(), group->end(), tmp.begin(), [](auto& item) {return &item; });
		}
		groupedPtr[i] = tmp;
		i++;
	}

	double minDeviation = 99999999999999999999.;
	int minGroup = 0;
	const QPointF* curr[6]{};
	const QPointF* bestPoints[6]{};


	for (const auto& g0 : groupedPtr[0]) {
		for (const auto& g1 : groupedPtr[1]) {
			for (const auto& g2 : groupedPtr[2]) {
				for (const auto& g3 : groupedPtr[3]) {
					for (const auto& g4 : groupedPtr[4]) {
						for (const auto& g5 : groupedPtr[5]) {
							curr[0] = g0;
							curr[1] = g1;
							curr[2] = g2;
							curr[3] = g3;
							curr[4] = g4;
							curr[5] = g5;


							// calculate average
							double sumptx = 0.;
							double sumpty = 0.;
							int n = 0;
							for (int point = 0; point < 6; point++) {
								if (!curr[point])
									continue;
								sumptx += curr[point]->x();
								sumpty += curr[point]->y();
								n++;
							}
							sumptx /= n;
							sumpty /= n;

							double deviation = 0.;
							// calculate deviation
							for (int point = 0; point < 6; point++) {
								if (!curr[point])
									continue;
								double x = curr[point]->x();
								double y = curr[point]->y();
								deviation += (sumptx - x) * (sumptx - x) + (sumpty - y) * (sumpty - y);
							}

							// determine if this is the best group so far
							if (deviation < minDeviation) {
								minDeviation = deviation;
								memcpy(bestPoints, curr, 6 * sizeof(QPointF*));
							}
						}
					}
				}
			}
		}
	}



	double sumptx = 0.;
	double sumpty = 0.;
	int n = 0;
	for (int point = 0; point < 6; point++) {
		if (!bestPoints[point])
			continue;
		double x = bestPoints[point]->x();
		double y = bestPoints[point]->y();
		sumptx += x;
		sumpty += y;
		n++;
	}
	if (n != 0)
	{
		sumptx /= n;
		sumpty /= n;
	}



	// rotate point
	double rangle = -m_cfg.portAngle() * (CV_PI / 180);
	double s = std::sin(rangle);
	double c = std::cos(rangle);
	double ty = sumpty + 0.5;
	double cx = sumptx * c - ty * s;
	double cy = sumptx * s + ty * c;
	cy -= 0.5;

	return QPointF(cx, cy);
}

std::unique_ptr<std::array<std::array<std::vector<QPointF>, 256>, 256>> MainWindow::precalculateIntersections(Segments seg1, Segments seg2, unsigned long& status, const bool& cancelPoint)
{
	auto res = make_unique<std::array<std::array<std::vector<QPointF>, 256>, 256>>();
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			auto el1 = Superellipse(m_estimator.getA(i), m_estimator.getB(i), m_estimator.getC(i), 0, m_estimator.getK(i));
			el1.changeSegment(seg1);
			auto el2 = Superellipse(m_estimator.getA(j), m_estimator.getB(j), m_estimator.getC(j), 0, m_estimator.getK(j));
			el2.changeSegment(seg2);
			res->at(i)[j] = el1.findPOIs(el2);
		}
		status++;
		if (cancelPoint == true) {
			return res;
		}
	}
	return res;
}


