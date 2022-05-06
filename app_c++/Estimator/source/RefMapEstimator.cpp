#include "RefMapEstimator.h"
#include <iostream>
#include <fstream>
#include <charconv>
#include <boost/algorithm/string.hpp>


using namespace std;

RefMapEstimator::RefMapEstimator(const double energy, const double current, const double wd) :
	m_bc(current), m_wd(wd), m_be(energy),
	m_a(range256(), load_a())
{
	pchip roi_l_pos(move(energy_range()), move(load_roi_l()));
	pchip roi_h_pos(move(energy_range()), move(load_roi_h()));
	m_roi_l = roi_l_pos(wd);
	m_roi_h = roi_h_pos(wd);
}

double RefMapEstimator::getA(int val)
{
	return 0.0;
}

double RefMapEstimator::getB(int val)
{
	return 0.0;
}

double RefMapEstimator::getC(int val)
{
	return 2.1;
}

double RefMapEstimator::getK(int val)
{
	return 0.0;
}

vector<double> RefMapEstimator::load_roi_l()
{
	vector<double> result(6 ,0.);

	auto file = ifstream(FILE_ROI_L);
	string line;
	for (int i = 0; i < 6; i++) {
		getline(file, line);
		double res = 0.;
		from_chars(line.c_str(), line.c_str()+line.size(), res);
		result[i] = res;
	}

	return result;
}

vector<double> RefMapEstimator::load_roi_h()
{
	vector<double> result(6, 0.);

	auto file = ifstream(FILE_ROI_H);
	string line;
	for (int i = 0; i < 6; i++) {
		getline(file, line);
		double res = 0.;
		from_chars(line.c_str(), line.c_str() + line.size(), res);
		result[i] = res;
	}

	return result;
}

//TODO cont
vector<double> RefMapEstimator::load_a()
{
	vector<double> result(N_POINTS+1,0.);

	string line;
	auto file = ifstream(FILE_ROI_H);

	for (int i = 0; i < N_POINTS+1; i++) {
		getline(file, line);
		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		for (const auto& s : v) {
			cout << s << " - ";
		}
		cout << '\n';
	}

	return result;
}



vector<double> RefMapEstimator::range256()
{
	vector<double> res(N_POINTS+1,0.);
	for (int i = 0; i < N_POINTS+1; i++) {
		res[i] = i * 8;
	}
	return res;
}

vector<double> RefMapEstimator::energy_range()
{
	return vector<double>({5,10,15,20,25,30});
}

vector<double> RefMapEstimator::wd_range()
{
	return vector<double>({10,12,14,16,18,20});
}
