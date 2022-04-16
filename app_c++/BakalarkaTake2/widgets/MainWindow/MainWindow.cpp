#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>


using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	setupUi(this);
	auto sizes = splitter->sizes();
	sizes[0] = 500;
	sizes[1] = 100;
	splitter->setSizes(sizes);

	connect(actionLoad_reflectance_maps, &QAction::triggered, this, &MainWindow::onLoadReflectanceMaps);
	connect(actionLoad_BSE_images, &QAction::triggered, this, &MainWindow::onLoadBSEImages);
	connect(actionShow_normal_image, &QAction::triggered, this, &MainWindow::onShowNormalImage);
	connect(imageManager->selector(), &ImageLabel::selected, this, &MainWindow::onSelected);
	connect(imageManager->segmentManager(), &SegmentManager::swapped, this, &MainWindow::onSwapped);

	// TODO remove
	try
	{
		loadReflectanceMaps("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/Q4-upravene-spravne/5kV-upravene-spravne/5kV_10mm_3_u.png");
		array<QString, 4> filenames = {
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_1.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_2.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_3.png",
				"C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/dataset/2022-04-11 data pro Sama konst GO/5 kV/5kV_10mm_4.png",
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
}

void MainWindow::loadReflectanceMaps(QString filename) {
	if (filename == "") {
		filename = QFileDialog::getOpenFileName();
	}

	if (filename.isEmpty()) {
		QMessageBox::critical(this, "Unable to open", "Reflectance map could not be loaded.");
		return;
	}

	// prepare BGR images
	m_maps[0] = imread(filename.toStdString());
	for (int i = 1; i < 4; i++) {
		cv::rotate(m_maps[i - 1], m_maps[i], ROTATE_90_CLOCKWISE);
	}

	// switch last 2 to keep logical order in UI
	swap(m_maps[2], m_maps[3]);

	// prepare grayscale images
	for (int i = 0; i < 4; i++) {
		cvtColor(m_maps[i], m_grayMaps[i], COLOR_BGR2GRAY);
	}

	reflectanceMap->setMap(m_maps, m_grayMaps);
	calculateMasks();
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
		m_imgs[i] = img;
		cvtColor(m_imgs[i], m_grayImgs[i], COLOR_BGR2GRAY);
	}

	imageManager->loadImages(m_imgs);
}

void MainWindow::showNormalImage()
{
	// TODO
}

void MainWindow::onSelected(double x, double y)
{
	int trueX = m_imgs[0].cols * x;
	int trueY = m_imgs[0].rows * y;
	auto a = m_grayImgs[0].at<uint8_t>(trueY, trueX);
	auto b = m_grayImgs[1].at<uint8_t>(trueY, trueX);
	auto c = m_grayImgs[2].at<uint8_t>(trueY, trueX);
	auto d = m_grayImgs[3].at<uint8_t>(trueY, trueX);
	reflectanceMap->colorPixels(a, b, c, d);

	Mat& mask0 = m_masks[0][m_grayImgs[0].at<uint8_t>(y, x)];
	Mat& mask1 = m_masks[1][m_grayImgs[1].at<uint8_t>(y, x)];
	Mat& mask2 = m_masks[2][m_grayImgs[2].at<uint8_t>(y, x)];
	Mat& mask3 = m_masks[3][m_grayImgs[3].at<uint8_t>(y, x)];

	Mat mask01, mask02, mask03, mask12, mask13, mask23;

	cv::bitwise_and(mask0, mask1, mask01);
	cv::bitwise_and(mask0, mask2, mask02);
	cv::bitwise_and(mask0, mask3, mask03);
	cv::bitwise_and(mask1, mask2, mask12);
	cv::bitwise_and(mask1, mask3, mask13);
	cv::bitwise_and(mask2, mask3, mask23);

	array<const Mat*, 6> masks = { &mask01, &mask02, &mask03, &mask12, &mask13, &mask23 };
	array<pair<int, int>, 6> maskIndices = { {{0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3}} };

	vector<Point> finalPoints;
	vector<Point> tentativePoints;

	for (int i = 0; i < 6; i++) {
		const Mat& mask = *masks[i];
		int height = mask.size().height;
		int width = mask.size().width;
		int up{}, right{}, down{}, left{};
		// normalization lambda
		auto norm = [height, width](Point& pt) { return Point2d(pt.x / (double)width, pt.y / (double)height); };

		Rect boundaries = boundingRect(mask);
		// if 
		if (boundaries.empty()) {
			findClosestPair(m_masks[i][maskIndices[i].first], m_masks[i][maskIndices[i].second]);
		}
		else {
			// find furthest points
		}

	}


}

void MainWindow::onSwapped(int first, int second)
{
	swap(m_imgs[first], m_imgs[second]);
	swap(m_grayImgs[first], m_grayImgs[second]);
}

void MainWindow::calculateMasks() {
	for (int n = 0; n < 4; n++) {
		auto& img = m_grayMaps[n];
		for (int i = 0; i < 256; i++) {
			auto& mask = m_masks[n][i];
			mask = Mat::zeros(img.size(), CV_8U);
			img.forEach<uint8_t>([i, &mask](uint8_t pixel, const int* position) 
				{
					if (pixel == i) mask.at<uint8_t>(position[0], position[1]) = 1;
				}
			);
		}
	}
}

cv::Point MainWindow::findClosestPair(const Mat& first, const Mat& second)
{
	return cv::Point();
}
