#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>


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
		loadReflectanceMaps("C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/sim2.png");
		array<QString, 4> filenames = {
				"../../../data/cinove_koule/25_mikro/cropped/25_mikro_6.tif",
				"../../../data/cinove_koule/25_mikro/cropped/25_mikro_5.tif",
				"../../../data/cinove_koule/25_mikro/cropped/25_mikro_3.tif",
				"../../../data/cinove_koule/25_mikro/cropped/25_mikro_4.tif",
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
	auto tmp = m_maps[2];
	m_maps[2] = m_maps[3];
	m_maps[3] = tmp;

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
		}
		for (int i = 0; i < 4; i++) {
			paths[i] = list[i];
		}
	}

	for (int i = 0; i < 4; i++) {
		m_imgs[i] = imread(paths[i].toStdString());
		if (m_imgs[i].empty()) {
			QMessageBox::critical(this, "Unable to open", "Image \"" + paths[i] + "\" could not be loaded.");
			return;
		}
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
	auto a = m_grayImgs[0].at<uint8_t>(trueX, trueY);
	auto b = m_grayImgs[1].at<uint8_t>(trueX, trueY);
	auto c = m_grayImgs[2].at<uint8_t>(trueX, trueY);
	auto d = m_grayImgs[3].at<uint8_t>(trueX, trueY);
	reflectanceMap->colorPixels(a, b, c, d);
}

void MainWindow::onSwapped(int first, int second)
{
	auto tmp = m_imgs[first];
	m_imgs[first] = m_imgs[second];
	m_imgs[second] = tmp;

	tmp = m_grayImgs[first];
	m_grayImgs[first] = m_grayImgs[second];
	m_grayImgs[second] = tmp;
}

void MainWindow::calculateMasks() {
	for (int n = 0; n < 4; n++) {
		auto img = m_grayMaps[n];
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
