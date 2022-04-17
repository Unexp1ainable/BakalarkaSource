#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>



using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), m_cfg()
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
	if (m_angleDialog) {
		delete m_angleDialog;
	}
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
	m_origMaps[0] = imread(filename.toStdString());
	for (int i = 1; i < 4; i++) {
		cv::rotate(m_origMaps[i - 1], m_origMaps[i], ROTATE_90_CLOCKWISE);
	}

	// switch last 2 to keep logical order in UI
	swap(m_origMaps[2], m_origMaps[3]);

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
	// TODO
}

void MainWindow::onDetectorSettingsChanged()
{
	processReflectanceMaps();
	processBSEImages();
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

}

void MainWindow::onSwapped(int first, int second)
{
	swap(m_imgs[first], m_imgs[second]);
	swap(m_grayImgs[first], m_grayImgs[second]);
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
