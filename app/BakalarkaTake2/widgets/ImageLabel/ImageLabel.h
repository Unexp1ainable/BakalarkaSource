#pragma once
/*****************************************************************//**
 * \file   ImageLabel.h
 * \brief  Resizable image widget
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>

#include <memory>
#include <opencv2/opencv.hpp>
#include "ui_ImageLabel.h"

class ImageLabel : public QLabel
{
	Q_OBJECT

public:
	ImageLabel(QWidget *parent = nullptr);
	~ImageLabel();

	void setMat(const cv::Mat& img);
	void setPixmap(const QPixmap& img);
	void setQImage(const QImage& img);

	QPixmap fullPixmap() { return m_pixmap; }

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	QPixmap m_pixmap = QPixmap();

private:
	Ui::ImageLabel ui;

signals:
	void selected(double x, double y);
};
