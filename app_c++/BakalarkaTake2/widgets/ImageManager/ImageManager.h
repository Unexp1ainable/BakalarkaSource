#pragma once

#include <QWidget>

#include <array>

#include "ui_ImageManager.h"

class ImageManager : public QWidget
{
	Q_OBJECT

public:
	ImageManager(QWidget *parent = nullptr);
	~ImageManager();

	const ImageLabel* selector() const { return ui.selector; }
	const SegmentManager* segmentManager() const { return ui.segmentManager; }

	void loadImages(std::array<cv::Mat, 4> imgs);

private:
	Ui::ImageManager ui;
};
