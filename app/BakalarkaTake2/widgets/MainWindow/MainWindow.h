#pragma once

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
	void loadBSEImages(std::array<QString, 4> paths = {""});
	void processBSEImages();
	void showNormalImage();
	QPointF evaluatePointsGraphic(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34);
	QPointF evaluatePoints(std::vector<QPointF>& npts12, std::vector<QPointF>& npts13, std::vector<QPointF>& npts14, std::vector<QPointF>& npts23, std::vector<QPointF>& npts24, std::vector<QPointF>& npts34);
	std::unique_ptr<std::array < std::array < std::vector<QPointF>,256>,256>> precalculateIntersections(Segments seg1, Segments seg2, unsigned long& status, const bool& cancelPoint);
	//std::vector<std::vector<QPointF>> generateFinalGroups(std::array<std::vector<QPointF>*, 6>& grouped, int lvl);
	
	std::array<std::array<cv::Mat, 256>, 4> m_masks;

	std::array<cv::Mat, 4> m_maps;
	std::array<cv::Mat, 4> m_origMaps;
	std::array<cv::Mat, 4> m_grayMaps;
						   
	std::array<cv::Mat, 4> m_imgs;
	std::array<cv::Mat, 4> m_origImgs;
	std::array<cv::Mat, 4> m_grayImgs;

	DetectorSettingsDialog* m_angleDialog = nullptr;

	Configuration m_cfg;
	RefMapEstimator m_estimator{};

	static constexpr int MAX_RES = 512;

protected slots:
	void onLoadBSEImages(bool checked) { loadBSEImages(); };
	void onShowNormalImage(bool checked) { showNormalImage(); };
	void onDetectorSettingsInvoked(bool checked);
	void onDetectorSettingsChanged();
	void onUseDefaultMaterial();
	void onSelected(double x, double y);
	void onSwapped(int first, int second);

};
