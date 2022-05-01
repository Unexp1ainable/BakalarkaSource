#pragma once

#include <QPointF>

#include <vector>

#include <boost/math/interpolators/pchip.hpp>
#include <opencv2/opencv.hpp>
#include "Configuration.h"


struct RootResult;

class Superellipse {
public:
    Superellipse() = default;
	Superellipse(double a, double b, double c, double h, double k);


    double a() const { return m_a; }
    double b() const { return m_b; }
    double c() const { return m_c; }
    double h() const { return m_h; }
    double k() const { return m_k; }
    std::pair<QPointF, QPointF> bBox() const { return m_bBox; };

    void setA(double a) { m_a = a; calculateBoundingBox(); }
    void setB(double b) { m_b = b; calculateBoundingBox(); }
    void setC(double c) { m_c = c; calculateBoundingBox(); }
    void setH(double h) { m_h = h; calculateBoundingBox(); }
    void setK(double k) { m_k = k; calculateBoundingBox(); }

    /**
     * @brief Transform ellipse from being bound to Q1 (default) to another segment. 
     * Works ONLY for transformation from Q1 to another.
     * @param seg Target segment
     * @param width Canvas width
     * @param height Canvas height
    */
    void changeSegment(Segments seg, int width = 1, int height = 1);
    void scale(int width, int height);

    RootResult rootsX(double y) const;
    RootResult rootsY(double x) const;
    QPointF autoRoots(double x, double y, bool& ok) const;

    std::vector<QPoint> rasterize() const;
    void draw(cv::Mat& img) const;
    std::vector<QPointF> borderPoints() const;
    std::vector<QPointF> findPOIs(const Superellipse& el2) const;

    bool isNull() const;

protected:
    void calculateBoundingBox();

    double m_a = 0.;
    double m_b = 0.;
    double m_c = 0.;
    double m_h = 0.;
    double m_k = 0.;

    std::pair<QPointF, QPointF> m_bBox;
	//boost::math::interpolators::pchip<std::vector<double>> a;
};

struct RootResult {
    int n;
    double r1;
    double r2;
};
