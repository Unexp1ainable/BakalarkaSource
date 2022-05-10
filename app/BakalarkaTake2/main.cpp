/*****************************************************************//**
 * \file   main.cpp
 * \brief  Entry point
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/

#include <iostream>
#include <QFile>
#include <QMainWindow>
#include <QApplication>
#include <opencv2/opencv.hpp>
#include "widgets/MainWindow/MainWindow.h"

#include <QVBoxLayout>

// Defines the entry point for the application.
int main(int argc, char* argv[])
{	
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
	auto app = QApplication(argc, argv);
	auto window = MainWindow();

	QFile file(":/gui/style/style.qss");
	file.open(QFile::ReadOnly);
	if (file.isOpen()) {
		QString styleSheet = QString(file.readAll());
		app.setStyleSheet(styleSheet);
	}

	window.resize(800, 800);
	window.show();
	return app.exec();
}
