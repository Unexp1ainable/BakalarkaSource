#pragma once

#include <boost/math/interpolators/pchip.hpp>
#include <vector>

using boost::math::interpolators::pchip;


class Interpolator {
public:
	Interpolator();

	double getA(int val) { return m_a(val); }
	double getB(int val) { return m_b(val); }
	double getC(int val) { return m_c; }
	double getK(int val) { return m_k(val); }

protected:
	pchip<std::vector<double>> m_a;
	pchip<std::vector<double>> m_b;
	double m_c = 2.1;
	pchip<std::vector<double>> m_k;
};
