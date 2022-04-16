#pragma once

#include <QMainWindow>
#include <QString>

#include <array>

#include <opencv2/opencv.hpp>
#include "ui_MainWindow.h"
#include "../../engine/Superellipse.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

protected:
	void loadReflectanceMaps(QString filename = "");
	void loadBSEImages(std::array<QString, 4> paths = {""});
	void showNormalImage();
	
	void calculateMasks();

	cv::Point findClosestPair(const cv::Mat& first, const cv::Mat& second);

	std::array<std::array<cv::Mat, 256>, 4> m_masks;

	std::array<cv::Mat, 4> m_maps;
	std::array<cv::Mat, 4> m_grayMaps;
						   
	std::array<cv::Mat, 4> m_imgs;
	std::array<cv::Mat, 4> m_grayImgs;



protected slots:
	void onLoadReflectanceMaps(bool checked) { loadReflectanceMaps(); };
	void onLoadBSEImages(bool checked) { loadBSEImages(); };
	void onShowNormalImage(bool checked) { showNormalImage(); };
	void onSelected(double x, double y);
	void onSwapped(int first, int second);

};
