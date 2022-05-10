#include "ReflectanceMap.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

using namespace std;
using namespace cv;

ReflectanceMap::ReflectanceMap(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_labels = { ui.map0Label, ui.map1Label, ui.map2Label, ui.map3Label };
}

ReflectanceMap::~ReflectanceMap()
{
}

void ReflectanceMap::setMap(array<Mat, 4>& maps, array<Mat, 4>& grayMaps)
{
	Mat sumMap = Mat::zeros(grayMaps[0].size(), CV_16UC3);

	for (int i = 0; i < 4; i++) {
		sumMap += maps[i];	// prepare sumMap
		m_maps[i] = QImage((unsigned char*)maps[i].data, maps[i].cols, maps[i].rows, QImage::Format_BGR888).copy();
		m_grayMaps[i] = QImage((unsigned char*)grayMaps[i].data, grayMaps[i].cols, grayMaps[i].rows, QImage::Format_Grayscale8).copy();
		m_labels[i]->setQImage(m_maps[i]);
	}
	sumMap /= 4;
	Mat tmp;
	sumMap.convertTo(tmp, CV_8UC3);
	m_sumMap = QImage((unsigned char*)tmp.data, tmp.cols, tmp.rows, QImage::Format_BGR888).copy();
	ui.mapOverviewLabel->setQImage(m_sumMap);
}

void ReflectanceMap::colorPixels(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	if (m_grayMaps[0].isNull())
		return;

	array<QColor, 4> colors = { QColor("#FF0000"), QColor("#80FF00"), QColor("#00FFFF"), QColor("#8000FF"), };
	array<unsigned char, 4> values = { a, b, c, d };
	auto sumCopy = m_sumMap.copy();


	for (int i = 0; i < 4; i++) {
		auto mask = m_grayMaps[i].createMaskFromColor(values[i], Qt::MaskOutColor);
		auto imgCopy = m_maps[i].copy();

		for (int y = 0; y < imgCopy.height(); y++)
		{
			for (int x = 0; x < imgCopy.width(); x++)
			{
				if ((m_grayMaps[i].pixel(x, y) & 0b11111111) == values[i]) {
					imgCopy.setPixelColor(x, y, colors[i]);
					sumCopy.setPixelColor(x, y, colors[i]);
				}
			}
		}

		m_labels[i]->setQImage(imgCopy);
	}
	ui.mapOverviewLabel->setQImage(sumCopy);
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

void ReflectanceMap::setPQ(double p, double q)
{
	ui.pqCoords->setText(QString::number(p) + ", " + QString::number(q));
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
