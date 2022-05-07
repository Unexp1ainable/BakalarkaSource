#pragma once

#include <boost/math/interpolators/pchip.hpp>
#include <vector>

using boost::math::interpolators::pchip;


class RefMapEstimator {
public:
	RefMapEstimator(const double energy, const double wd, const double current);
	RefMapEstimator() : m_a({ 0,1,2,3 }, { 0,0,0,0 }), m_b({ 0,1,2,3 }, { 0,0,0,0 }), m_k({ 0,1,2,3 }, { 0,0,0,0 }) {}

	void reset(const double energy, const double wd, const double current);

	double getA(int val);
	double getB(int val);
	double getC(int val);
	double getK(int val);
	
protected:
	double load_roi_l();
	double load_roi_h();
	static std::vector<double> load_a(const double be);
	static std::vector<double> load_b(const double wd);
	static std::vector<double> load_k(const double wd);

	static std::vector<double> range(double start, double stop, double step=1.);
	static std::vector<double> energy_range();
	static std::vector<double> wd_range();


	double m_be;
	double m_wd;
	double m_bc;

	double m_roi_l;
	double m_roi_h;
	double m_roi_diff;

	// --------- pchips ---------
	pchip<std::vector<double>> m_a;
	pchip<std::vector<double>> m_b;
	pchip<std::vector<double>> m_k;


	// -------- FILES --------
	static inline const constexpr char* FILE_ROI_L = "../../BakalarkaTake2/data/roi_l.csv";
	static inline const constexpr char* FILE_ROI_H = "../../BakalarkaTake2/data/roi_h.csv";
	static inline const constexpr char* FILE_A =	 "../../BakalarkaTake2/data/a.csv";
	static inline const constexpr char* FILE_B =	 "../../BakalarkaTake2/data/b.csv";
	static inline const constexpr char* FILE_K =	 "../../BakalarkaTake2/data/k.csv";
	
	static inline const constexpr int N_POINTS = 32;
	static inline const constexpr int N_POINTS_BE = 6;
	static inline const constexpr int N_POINTS_WD = 6;
	static inline const constexpr double BASE_BC = 5.;
};