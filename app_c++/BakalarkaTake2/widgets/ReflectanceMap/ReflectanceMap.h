#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>

#include <memory>
#include <vector>
#include <array>

#include <opencv2/opencv.hpp>
#include "ui_ReflectanceMap.h"

class ReflectanceMap : public QWidget
{
	Q_OBJECT

public:
	ReflectanceMap(QWidget *parent = nullptr);
	~ReflectanceMap();

	void setMap(std::array<cv::Mat, 4>& maps, std::array<cv::Mat, 4>& grayMaps);
	void colorPixels(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
	void point(double x, double y, QColor = Qt::magenta);
	void setPQ(double p, double q);

protected:
	std::array<ImageLabel*, 4> m_labels;

	std::array<QImage, 4> m_maps;
	std::array<QImage, 4> m_grayMaps;
	QImage m_sumMap;

private:
	Ui::ReflectanceMap ui;
};
