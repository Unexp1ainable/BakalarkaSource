/*****************************************************************//**
 * \file   ReflectanceMap.cpp
 * \brief  Implementation fo the class
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include "ReflectanceMap.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QSvgRenderer>

using namespace std;
using namespace cv;

ReflectanceMap::ReflectanceMap(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_labels = { ui.map0Label, ui.map1Label, ui.map2Label, ui.map3Label };

	for (int i = 0; i < 4; i++) {
		QSvgRenderer renderer(QString(":/maps/svg/map.svg"));
		QImage image(600, 600, QImage::Format_ARGB32);
		QPainter painter(&image);
		renderer.render(&painter);

		QTransform transform = QTransform();
		transform.rotate(90 * i);
		m_maps[i] = image.transformed(transform);
		m_labels[i]->setQImage(m_maps[i]);
	}

	Mat sumMap = Mat::zeros(Size(800,800), CV_8UC3);
	circle(sumMap, Point(sumMap.size().width / 2, sumMap.size().height / 2), sumMap.size().height / 2, Scalar(120, 120, 120), -1);
	m_sumMap = QImage((unsigned char*)sumMap.data, sumMap.cols, sumMap.rows, QImage::Format_BGR888).copy();
	ui.mapOverviewLabel->setQImage(m_sumMap);
}


ReflectanceMap::~ReflectanceMap()
{
}


void ReflectanceMap::point(double x, double y, QColor color)
{
	auto pix = ui.mapOverviewLabel->pixmap();
	if (pix.isNull())
		return;

	auto painter = QPainter(&pix);
	auto pen = QPen(color, 3);
	auto brush = QBrush(color);
	painter.setPen(pen);
	painter.setBrush(brush);
	painter.drawEllipse(QPoint(static_cast<int>(x * pix.width()), static_cast<int>(y * pix.height())), 3, 3);
	painter.end();
	ui.mapOverviewLabel->setPixmap(pix);
}


void ReflectanceMap::drawSuperellipse(Superellipse el, Segments seg)
{
	if (ui.map0Label->pixmap().isNull())
		return;
	auto imgOverview = ui.mapOverviewLabel->pixmap().toImage();
	QImage img;

	if (seg == Segments::Q1) {
		img = ui.map0Label->pixmap().toImage();
	}
	else if (seg == Segments::Q2) {
		img = ui.map1Label->pixmap().toImage();
	}
	else if (seg == Segments::Q3) {
		img = ui.map2Label->pixmap().toImage();
	}
	else {
		img = ui.map3Label->pixmap().toImage();
	}

	int h = img.height();
	int w = img.width();
	int wh = w / 2;
	Superellipse tmp = el;
	tmp.scale(w, h);
	auto pts = tmp.rasterize();
	for (auto pt : pts) {
		pt = QPoint(pt.x() + wh, -pt.y());
		if (0 <= pt.x() && pt.x() < w && 0 <= pt.y() && pt.y() < h)
			img.setPixelColor(pt, Qt::white);
	}
	h = imgOverview.height();
	w = imgOverview.width();
	wh = w / 2;
	tmp = el;
	tmp.scale(w, h);
	pts = tmp.rasterize();
	for (auto pt : pts) {
		pt = QPoint(pt.x() + wh, -pt.y());
		if (0 <= pt.x() && pt.x() < w && 0 <= pt.y() && pt.y() < h)
			imgOverview.setPixelColor(pt.x(), pt.y(), Qt::white);
	}

	if (seg == Segments::Q1) {
		ui.map0Label->setQImage(img);
	}
	else if (seg == Segments::Q2) {
		ui.map1Label->setQImage(img);
	}
	else if (seg == Segments::Q3) {
		ui.map2Label->setQImage(img);
	}
	else {
		ui.map3Label->setQImage(img);
	}

	ui.mapOverviewLabel->setQImage(imgOverview);
}

void ReflectanceMap::reset()
{
	for (int i = 0; i < 4; i++) {
		m_labels[i]->setQImage(m_maps[i]);
	}
	ui.mapOverviewLabel->setQImage(m_sumMap);
}
