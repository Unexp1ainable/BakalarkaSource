#pragma once
/*****************************************************************//**
 * \file   Superellipse.h
 * \brief  Superellipse class
 * 
 * \author Samuel Repka
 * \date   May 2022
 *********************************************************************/
#include <QPointF>

#include <vector>

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
    // bounding box
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

    /**
     * Scale normalised superellipse to given width and height.
     */
    void scale(int width, int height);

    // calculate superellipse points in the given x or y coordinate
    RootResult rootsX(double y) const;
    RootResult rootsY(double x) const;
    QPointF autoRoots(double x, double y, bool& ok) const;

    /**
     * Rasterize superellipse.
     */
    std::vector<QPoint> rasterize() const;

    /**
     * Draw superellipse onto the image.
     */
    void draw(cv::Mat& img) const;

    /**
     * Rasterize the superellipse beginning from the top middle, continuing to the right around the whole superellipse.
     */
    std::vector<QPointF> borderPoints() const;

    /**
     * Find points of interest relative to another superellipse.
     */
    std::vector<QPointF> findPOIs(const Superellipse& el2) const;

    /**
     * Check if the superellipse is not a point.
     */
    bool isNull() const;

protected:
    void calculateBoundingBox();

    double m_a = 0.;
    double m_b = 0.;
    double m_c = 0.;
    double m_h = 0.;
    double m_k = 0.;

    std::pair<QPointF, QPointF> m_bBox;
};

struct RootResult {
    int n;
    double r1;
    double r2;
};
