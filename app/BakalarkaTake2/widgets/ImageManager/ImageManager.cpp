#include "ImageManager.h"

using namespace std;
using namespace cv;

ImageManager::ImageManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ImageManager::~ImageManager()
{
}

void ImageManager::loadImages(array<Mat, 4> imgs)
{
	ui.segmentManager->loadImages(imgs);

	Mat summed = Mat::zeros(imgs[0].size(), CV_16UC3);
	for (const auto& img : imgs) {
		summed += img;
	}
	summed /= 4;
	summed.convertTo(summed, CV_8UC3);

	ui.selector->setMat(summed);
}
