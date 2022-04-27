#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <iostream>
#include <fstream>
#include <future>
#include <chrono>
#include <thread>


using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), m_cfg(), m_ipt()
{
	setupUi(this);
	auto sizes = splitter->sizes();
	sizes[0] = 500;
	sizes[1] = 100;
	splitter->setSizes(sizes);

	connect(actionLoad_reflectance_maps, &QAction::triggered, this, &MainWindow::onLoadReflectanceMaps);
	connect(actionLoad_BSE_images, &QAction::triggered, this, &MainWindow::onLoadBSEImages);
	connect(actionShow_normal_image, &QAction::triggered, this, &MainWindow::onShowNormalImage);
	connect(actionPort_angle, &QAction::triggered, this, &MainWindow::onDetectorSettingsInvoked);
	connect(imageManager->selector(), &ImageLabel::selected, this, &MainWindow::onSelected);
	connect(imageManager->segmentManager(), &SegmentManager::swapped, this, &MainWindow::onSwapped);

	// TODO remove
	try
	{
		loadReflectanceMaps("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/testmaps/5kV_10mm_3_u.png");
		array<QString, 4> filenames = {
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_1.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_2.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_4.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_3.png",
		};
		loadBSEImages(filenames);
	}
	catch (const std::exception& e)
	{
		qDebug("Sample data initialization failed.");
		qDebug(e.what());
	}
}

MainWindow::~MainWindow()
{
	if (m_angleDialog) {
		delete m_angleDialog;
	}
}

void MainWindow::loadReflectanceMaps(QString filename) {
	if (filename == "") {
		filename = QFileDialog::getOpenFileName();
	}

	if (filename.isEmpty()) {
		return;
	}

	// prepare BGR images
	m_origMaps[0] = imread(filename.toStdString());
	if (m_origMaps[0].empty()) {
		QMessageBox::critical(this, "Unable to open", "Reflectance map could not be loaded.");
		return;
	}

	for (int i = 1; i < 4; i++) {
		cv::rotate(m_origMaps[i - 1], m_origMaps[i], ROTATE_90_COUNTERCLOCKWISE);
	}

	processReflectanceMaps();
}

void MainWindow::processReflectanceMaps()
{
	for (int i = 0; i < 4; i++) {
		//rotate images
		m_origMaps[i].copyTo(m_maps[i]);
		cv::Point2f pc(m_maps[i].cols / 2., m_maps[i].rows / 2.);
		cv::Mat r = cv::getRotationMatrix2D(pc, m_cfg.portAngle(), 1.0);

		cv::warpAffine(m_maps[i], m_maps[i], r, m_maps[i].size());
		// apply brightness contrast
		//cv::convertScaleAbs(m_maps[i], m_maps[i], m_cfg.alpha(i), m_cfg.beta(i));
	}

	// prepare grayscale images
	for (int i = 0; i < 4; i++) {
		cvtColor(m_maps[i], m_grayMaps[i], COLOR_BGR2GRAY);
	}

	reflectanceMap->setMap(m_maps, m_grayMaps);
}

void MainWindow::loadBSEImages(std::array<QString, 4> paths)
{
	if (paths[0] == "") {
		auto list = QFileDialog::getOpenFileNames();
		if (list.size() != 4) {
			QMessageBox::critical(this, "Unable to open", "Please select 4 images.");
			return;
		}
		for (int i = 0; i < 4; i++) {
			paths[i] = list[i];
		}
	}

	for (int i = 0; i < 4; i++) {
		auto img = imread(paths[i].toStdString());
		if (img.empty()) {
			QMessageBox::critical(this, "Unable to open", "Image \"" + paths[i] + "\" could not be loaded.");
			return;
		}

		// look for .hdr file
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
			}
		}
		m_origImgs[i] = img;
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

	auto file = std::ofstream("stuff");
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
				mid = 0.999;
			}
			double z = sqrt(1 - mid);
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

	cv::imwrite("final.png", finImg);
	cv::imshow("Normal map", finImg);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

void MainWindow::onDetectorSettingsChanged()
{
	m_cfg.save();
	processReflectanceMaps();
	processBSEImages();
}


void MainWindow::onSelected(double x, double y)
{
	int trueX = m_imgs[0].cols * x;
	int trueY = m_imgs[0].rows * y;
	cout << trueX << ", " << trueY << endl;
	int a = m_grayImgs[0].at<uint8_t>(trueY, trueX);
	int b = m_grayImgs[1].at<uint8_t>(trueY, trueX);
	int c = m_grayImgs[2].at<uint8_t>(trueY, trueX);
	int d = m_grayImgs[3].at<uint8_t>(trueY, trueX);
	reflectanceMap->colorPixels(a, b, c, d);

	auto el1 = Superellipse(m_ipt.getA(a), m_ipt.getB(a), m_ipt.getC(a), 0, m_ipt.getK(a));
	reflectanceMap->drawSuperellipse(el1, Segments::Q1);
	auto el2 = Superellipse(m_ipt.getA(b), m_ipt.getB(b), m_ipt.getC(b), 0, m_ipt.getK(b));
	el2.changeSegment(Segments::Q2);
	reflectanceMap->drawSuperellipse(el2, Segments::Q2);
	auto el3 = Superellipse(m_ipt.getA(c), m_ipt.getB(c), m_ipt.getC(c), 0, m_ipt.getK(c));
	el3.changeSegment(Segments::Q3);
	reflectanceMap->drawSuperellipse(el3, Segments::Q3);
	auto el4 = Superellipse(m_ipt.getA(d), m_ipt.getB(d), m_ipt.getC(d), 0, m_ipt.getK(d));
	el4.changeSegment(Segments::Q4);
	reflectanceMap->drawSuperellipse(el4, Segments::Q4);

	auto npts12 = el1.findPOIs(el2);
	auto npts13 = el1.findPOIs(el3);
	auto npts14 = el1.findPOIs(el4);
	auto npts23 = el2.findPOIs(el3);
	auto npts24 = el2.findPOIs(el4);
	auto npts34 = el3.findPOIs(el4);

	//assert(npts12.size() != 0);
	//assert(npts13.size() != 0);
	//assert(npts14.size() != 0);
	//assert(npts23.size() != 0);
	//assert(npts24.size() != 0);
	//assert(npts34.size() != 0);

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

	// check if at least one POI was found
	if (pts.size() == 0) {
		return QPointF(0., 0.);
	}

	double sumptx = 0.;
	double sumpty = 0.;
	for (const auto& pt : pts) {
		reflectanceMap->point(pt.x() + 0.5, -pt.y());
		sumptx += pt.x();
		sumpty += pt.y();
	}
	sumptx /= pts.size();
	sumpty /= pts.size();

	std::array<std::vector<QPointF>*, 6> grouped = { &npts12, &npts13, &npts14, &npts23, &npts24, &npts34 };
	std::array<bool, 6> flags;
	flags.fill(false);
	std::array<QPointF, 6> finals{};

	// pre-save solitary points
	for (int i = 0; i < 6; i++) {
		if (grouped[i]->size() == 0) {
			flags[i] = true;
		}
		else if (grouped[i]->size() == 1) {
			finals[i] = (*grouped[i])[0];
			flags[i] = true;
		}
	}

	// remove too distant points
	for (int i = 0; i < 6; i++) {
		if (flags[i])
			continue;
		double lowest = 1.;
		for (const auto& [x, y] : *grouped[i]) {
			double tx = x - sumptx;
			double ty = y - sumpty;
			double res = tx * tx + ty * ty;
			if (lowest > res) {
				lowest = res;
			}
		}
		lowest = sqrt(lowest);
		for (int j = 0; j < (*grouped[i]).size(); j++) {
			if (sqrt(pow((*grouped[i])[j].x() - sumptx, 2) + pow((*grouped[i])[j].y() - sumpty, 2)) - lowest > 0.1) {
				grouped[i]->erase(grouped[i]->begin() + j);
			}
		}

		// check if solitary
		if (grouped[i]->size() == 1) {
			finals[i] = (*grouped[i])[0];
			flags[i] = true;
		}
	}

	// recalculate midpoint
	sumptx = 0.;
	sumpty = 0.;
	int n = 0;
	for (const auto group : grouped) {
		for (const auto& [x, y] : *group) {
			sumptx += x;
			sumpty += y;
			n++;
		}
	}
	sumptx /= n;
	sumpty /= n;

	// final classification
	for (int i = 0; i < 6; i++) {
		if (flags[i])
			continue;

		double lowest = 1.;
		QPointF lowestPt;
		for (const auto& pt : *grouped[i]) {
			double tx = pt.x() - sumptx;
			double ty = pt.y() - sumpty;
			double res = tx * tx + ty * ty;
			if (lowest > res) {
				lowest = res;
				lowestPt = pt;
			}
		}
		finals[i] = lowestPt;

		// TODO close points handling
	}
	sumptx = 0.;
	sumpty = 0.;
	for (const auto& [x, y] : finals) {
		reflectanceMap->point(x + 0.5, -y, Qt::cyan);
		sumptx += x;
		sumpty += y;
	}
	sumptx /= 6;
	sumpty /= 6;

	reflectanceMap->point(sumptx + 0.5, -sumpty, Qt::white);

	double rangle = -m_cfg.portAngle() * (CV_PI / 180);

	double s = sin(rangle);
	double c = cos(rangle);

	// rotate point
	double ty = sumpty + 0.5;
	double cx = sumptx * c - ty * s;
	double cy = sumptx * s + ty * c;

	cy -= 0.5;

	reflectanceMap->point(cx + 0.5, -cy, Qt::darkGreen);

	return QPointF(sumptx, sumpty);
}

QPointF MainWindow::evaluatePoints(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34)
{
	std::array<std::vector<QPointF>*, 6> grouped = { &npts12, &npts13, &npts14, &npts23, &npts24, &npts34 };
	int nEmpty = 0;
	std::array<bool, 6> flags;
	flags.fill(false);
	std::array<QPointF, 6> finals;
	finals.fill(QPointF(0., 0.));


	// check if at least one POI was found
	for (int i = 0; i < 6; i++) {
		if ((*grouped[i]).size() == 0) {
			flags[i] = true;
		}
		else {
			nEmpty++;
		}
	}
	if (nEmpty == 0) {
		return QPointF(0., 0.);
	}

	double sumptx = 0.;
	double sumpty = 0.;
	int n = 0;
	for (const auto group : grouped) {
		for (const auto& [x, y] : *group) {
			sumptx += x;
			sumpty += y;
			n++;
		}
	}
	sumptx /= n;
	sumpty /= n;

	// pre-save solitary points
	for (int i = 0; i < 6; i++) {
		if (grouped[i]->size() == 0) {
			flags[i] = true;
		}
		else if (grouped[i]->size() == 1) {
			finals[i] = (*grouped[i])[0];
			flags[i] = true;
		}
	}

	// remove too distant points
	for (int i = 0; i < 6; i++) {
		if (flags[i])
			continue;
		double lowest = 1.;
		for (const auto& [x, y] : *grouped[i]) {
			double tx = x - sumptx;
			double ty = y - sumpty;
			double res = tx * tx + ty * ty;
			if (lowest > res) {
				lowest = res;
			}
		}
		lowest = sqrt(lowest);
		for (int j = 0; j < (*grouped[i]).size(); j++) {
			if (sqrt(pow((*grouped[i])[j].x() - sumptx, 2) + pow((*grouped[i])[j].y() - sumpty, 2)) - lowest > 0.1) {
				grouped[i]->erase(grouped[i]->begin() + j);
			}
		}

		// check if solitary
		if (grouped[i]->size() == 1) {
			finals[i] = (*grouped[i])[0];
			flags[i] = true;
		}
	}

	// recalculate midpoint
	sumptx = 0.;
	sumpty = 0.;
	n = 0;
	for (const auto group : grouped) {
		for (const auto& [x, y] : *group) {
			sumptx += x;
			sumpty += y;
			n++;
		}
	}
	sumptx /= n;
	sumpty /= n;

	// final classification
	for (int i = 0; i < 6; i++) {
		if (flags[i])
			continue;

		double lowest = 1.;
		QPointF lowestPt;
		for (const auto& pt : *grouped[i]) {
			double tx = pt.x() - sumptx;
			double ty = pt.y() - sumpty;
			double res = tx * tx + ty * ty;
			if (lowest > res) {
				lowest = res;
				lowestPt = pt;
			}
		}
		finals[i] = lowestPt;

		// TODO close points handling
	}

	sumptx = 0.;
	sumpty = 0.;
	for (const auto& [x, y] : finals) {
		sumptx += x;
		sumpty += y;
	}
	sumptx /= nEmpty;
	sumpty /= nEmpty;

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
			auto el1 = Superellipse(m_ipt.getA(i), m_ipt.getB(i), m_ipt.getC(i), 0, m_ipt.getK(i));
			el1.changeSegment(seg1);
			auto el2 = Superellipse(m_ipt.getA(j), m_ipt.getB(j), m_ipt.getC(j), 0, m_ipt.getK(j));
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
