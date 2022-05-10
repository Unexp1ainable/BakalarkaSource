/*****************************************************************//**
 * \file   ImageLabel.cpp
 * \brief  Implementation of the resizable item widget.
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include "ImageLabel.h"

using namespace std;
using namespace cv;

ImageLabel::ImageLabel(QWidget* parent)
	: QLabel(parent)
{
	ui.setupUi(this);
}

ImageLabel::~ImageLabel()
{
}

void ImageLabel::setMat(const Mat& img)
{
	setFrameStyle(QFrame::NoFrame);
	m_pixmap = QPixmap::fromImage(QImage((unsigned char*)img.data, img.cols, img.rows, QImage::Format_BGR888));
	QLabel::setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio));
}

void ImageLabel::setPixmap(const QPixmap& img)
{
	setFrameStyle(QFrame::NoFrame);
	m_pixmap = img;
	QLabel::setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio));
}

void ImageLabel::setQImage(const QImage& img)
{
	setFrameStyle(QFrame::NoFrame);
	m_pixmap = QPixmap::fromImage(img);
	QLabel::setPixmap(m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio));
}


void ImageLabel::resizeEvent(QResizeEvent* event)
{
	if (m_pixmap.isNull()) {
		return;
	}

	auto size = event->size();
	QLabel::setPixmap(m_pixmap.scaled(size, Qt::KeepAspectRatio));
}

void ImageLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		double x = event->position().x() - ((width() - pixmap().width()) / 2);
		double y = event->position().y() - ((height() - pixmap().height()) / 2);

		if (x < 0 or x >pixmap().width() or y < 0 or y >pixmap().height()) {
			return;
		}

		emit selected(x / pixmap().width(), y / pixmap().height());
	}
}
