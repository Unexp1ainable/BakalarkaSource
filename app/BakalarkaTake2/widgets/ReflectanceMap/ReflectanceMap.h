#pragma once
/*****************************************************************//**
 * \file   ReflectanceMap.h
 * \brief  Widget visualising calculation of the normals.
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include <QWidget>
#include <QImage>
#include <QColor>

#include <memory>
#include <vector>
#include <array>

#include <opencv2/opencv.hpp>
#include "ui_ReflectanceMap.h"
#include "../../engine/Superellipse.h"
#include "../../engine/Configuration.h"

class ReflectanceMap : public QWidget
{
	Q_OBJECT

public:
	ReflectanceMap(QWidget *parent = nullptr);
	~ReflectanceMap();

	/**
	 * Draw a point. Parameters are normalised.
	 */
	void point(double x, double y, QColor = Qt::magenta);
	/**
	 * Draw superellipse to the specified segment
	 */
	void drawSuperellipse(Superellipse el, Segments seg);

	/**
	 * Clear everything drawn.
	 */
	void reset();

protected:
	std::array<ImageLabel*, 4> m_labels;
	std::array<QImage, 4> m_maps;
	QImage m_sumMap;

private:
	Ui::ReflectanceMap ui;
};
