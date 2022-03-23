// BakalarkaTake2.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <QMainWindow>
#include <QApplication>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[])
{
	auto app = QApplication(argc, argv);
	auto window = QMainWindow();
  /*  # load style
        try :
        with open("style.qss") as file :
    lines = file.read()
        app.setStyleSheet(lines)
        except :
        pass*/
    auto b = cv::Mat::zeros(2,2,CV_8U);

    window.resize(800, 800);
    window.show();
    return app.exec();
}
