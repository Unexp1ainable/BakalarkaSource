#pragma once

#include <QWidget>
#include <QSvgWidget>

#include <array>

#include <opencv2/opencv.hpp>
#include "ui_SegmentManager.h"

class SegmentManager : public QWidget, public Ui_SegmentManager
{
	static constexpr int NOT_MOVING = -1;
	
	Q_OBJECT

public:
	SegmentManager(QWidget* parent = nullptr);
	~SegmentManager();

	void loadImages(std::array<cv::Mat, 4> imgs);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

protected:
	void swapImages(int first, int second);

	std::array<ImageLabel*, 4> m_labels;

private:
	//Ui::SegmentManager ui;
	int m_moving = NOT_MOVING;

signals:
	void swapped(int first, int second);
};
