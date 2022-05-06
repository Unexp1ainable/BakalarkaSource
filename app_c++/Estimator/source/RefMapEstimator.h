#pragma once

#include <boost/math/interpolators/pchip.hpp>
#include <vector>

using boost::math::interpolators::pchip;


class RefMapEstimator {
public:
	RefMapEstimator(const double energy, const double current, const double wd);

	double getA(int val);
	double getB(int val);
	double getC(int val);
	double getK(int val);
	
protected:
	static std::vector<double> load_roi_l();
	static std::vector<double> load_roi_h();
	static std::vector<double> load_a();

	static std::vector<double> range256();
	static std::vector<double> energy_range();
	static std::vector<double> wd_range();


	const double m_be;
	const double m_wd;
	const double m_bc;

	double m_roi_l;
	double m_roi_h;

	// --------- pchips ---------
	const pchip<std::vector<double>> m_a;


	// -------- FILES --------
	static inline const constexpr char* FILE_ROI_L = "data/roi_l";
	static inline const constexpr char* FILE_ROI_H = "data/roi_h";
	static inline const constexpr char* FILE_ROI_A = "data/a";
	static inline const constexpr char* FILE_ROI_B = "data/b";
	static inline const constexpr char* FILE_ROI_K = "data/k";
	
	static inline const constexpr int N_POINTS = 32;
};