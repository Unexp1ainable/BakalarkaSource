#pragma once
/*****************************************************************//**
 * \file   MainWindow.h
 * \brief  Main window of the software.
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/

#include <QDomDocument>
#include <QMainWindow>
#include <QString>

#include <array>
#include <memory>

#include <opencv2/opencv.hpp>
#include "ui_MainWindow.h"
#include "../../engine/Superellipse.h"
#include "../../engine/Configuration.h"
#include "../../engine/RefMapEstimator.h"
#include "../DetectorSettingsDialog/DetectorSettingsDialog.h"


class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	/**
	 * Load images with the given paths. If no paths are supplied, the user is prompted with a file dialog.
	 */
	void loadBSEImages(std::array<QString, 4> paths = {""});

	/**
	 * Process loaded images according to the calibration.
	 */
	void processBSEImages();

	/**
	 * Calculate normals over the whole image. The user is prompted to choose the save location of the normal file.
	 */
	void showNormalImage();

	/**
	 * Used when doubleclick. Input parameters are the points of interest from the ellipse pairs.
	 */
	QPointF evaluatePointsGraphic(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34);
	
	/**
	 * Used when calculating the normal image. Input parameters are the points of interest from the ellipse pairs.
	 */
	QPointF evaluatePoints(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34);
	
	/**
	 * Intersection precalculation routine.
	 */
	std::unique_ptr<std::array < std::array < std::vector<QPointF>,256>,256>> precalculateIntersections(Segments seg1, Segments seg2, unsigned long& status, const bool& cancelPoint);
	
	std::array<std::array<cv::Mat, 256>, 4> m_masks;

	std::array<cv::Mat, 4> m_maps;
	std::array<cv::Mat, 4> m_origMaps;
	std::array<cv::Mat, 4> m_grayMaps;
						   
	std::array<cv::Mat, 4> m_imgs;
	std::array<cv::Mat, 4> m_origImgs;
	std::array<cv::Mat, 4> m_grayImgs;

	DetectorSettingsDialog* m_angleDialog = nullptr;

	// global configuration
	Configuration m_cfg;
	// estimator of the ellipse parameters
	RefMapEstimator m_estimator{};

	static constexpr int MAX_RES = 512;

protected slots:
	void onLoadBSEImages(bool checked) { loadBSEImages(); };
	void onShowNormalImage(bool checked) { showNormalImage(); };
	void onDetectorSettingsInvoked(bool checked);
	void onDetectorSettingsChanged();
	void onUseDefaultMaterial();

	// select point on the sum image to calculate normal in point
	void onSelected(double x, double y);
	// swap segments
	void onSwapped(int first, int second);

};
