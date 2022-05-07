#include "RefMapEstimator.h"
#include <iostream>
#include <fstream>
#include <charconv>
#include <boost/algorithm/string.hpp>
#include <cmath>

using namespace std;

RefMapEstimator::RefMapEstimator(const double energy, const double wd, const double current) : 
m_bc(current), m_wd(wd), m_be(energy),
m_a(range(0, 256 + 1, 8), load_a(energy)),
m_b(range(0, 256 + 1, 8), load_b(wd)),
m_k(range(0, 256 + 1, 8), load_k(wd))
{
	// calculate roi
	m_roi_l = load_roi_l();
	m_roi_h = load_roi_h();

	// compensate for a different current
	m_roi_l /= BASE_BC / current;
	m_roi_h /= BASE_BC / current;

	m_roi_diff = m_roi_h - m_roi_l;
}

void RefMapEstimator::reset(const double energy, const double wd, const double current)
{
	m_bc = current;
	m_wd = clamp(wd, 10., 20.);
	m_be = clamp(energy, 5., 30.);
	m_a = pchip(range(0, 256 + 1, 8), load_a(m_be));
	m_b = pchip(range(0, 256 + 1, 8), load_b(m_wd));
	m_k = pchip(range(0, 256 + 1, 8), load_k(m_wd));

	// calculate roi
	m_roi_l = load_roi_l();
	m_roi_h = load_roi_h();

	// compensate for a different current
	m_roi_l /= BASE_BC / current;
	m_roi_h /= BASE_BC / current;
	m_roi_diff = m_roi_h - m_roi_l;
}

double RefMapEstimator::getA(int val)
{
	return std::clamp(m_a(std::clamp((val - m_roi_l) * 256/m_roi_diff,0.,256.)),0.01,200.);
}

double RefMapEstimator::getB(int val)
{
	return std::clamp(m_b(std::clamp((val - m_roi_l) * 256/m_roi_diff,0.,256.)),0.01,200.);
}

double RefMapEstimator::getC(int val)
{
	return 2.1;
}

double RefMapEstimator::getK(int val)
{
	return std::clamp(m_k(std::clamp((val - m_roi_l) * 256/m_roi_diff,0.,256.)),0.01,200.);
}

double RefMapEstimator::load_roi_l()
{
	auto file = ifstream(FILE_ROI_L);
	if (!file.is_open()) {
		throw std::exception("Could not load data/roi_l file.");
	}

	vector<double> aData(N_POINTS_WD, 0.);
	vector<double> bData(N_POINTS_WD, 0.);
	string line;
	for (int i = 0; i < N_POINTS_WD; i++) {
		getline(file, line);

		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		if (v.size() != 2) {
			throw exception("Wrong data for roil.");
		}
		from_chars(v[0].c_str(), v[0].c_str() + line.size(), aData[i]);
		from_chars(v[1].c_str(), v[1].c_str() + line.size(), bData[i]);
	}
	pchip aPchip(wd_range(), move(aData));
	pchip bPchip(wd_range(), move(bData));
	double trueA = aPchip(m_wd);
	double trueB = bPchip(m_wd);

	double result = trueA * m_be + trueB;
	return result;
}

double RefMapEstimator::load_roi_h()
{
	auto file = ifstream(FILE_ROI_H);
	if (!file.is_open()) {
		throw std::exception("Could not load data/roi_h file.");
	}

	vector<double> aData(N_POINTS_WD, 0.);
	vector<double> bData(N_POINTS_WD, 0.);
	string line;
	for (int i = 0; i < N_POINTS_WD; i++) {
		getline(file, line);

		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		if (v.size() != 2) {
			throw exception("Wrong data for roih.");
		}
		from_chars(v[0].c_str(), v[0].c_str() + line.size(), aData[i]);
		from_chars(v[1].c_str(), v[1].c_str() + line.size(), bData[i]);
	}
	pchip aPchip(wd_range(), move(aData));
	pchip bPchip(wd_range(), move(bData));
	double trueA = aPchip(m_wd);
	double trueB = bPchip(m_wd);

	double result = trueA * m_be + trueB;
	return result;
}


vector<double> RefMapEstimator::load_a(const double be)
{
	vector<double> result(N_POINTS + 1, 0.);

	string line;
	auto file = ifstream(FILE_A);
	if (!file.is_open()) {
		throw std::exception("Could not load data/a.csv file.");
	}

	for (int i = 0; i < N_POINTS + 1; i++) {
		getline(file, line);
		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		auto intermediary = vector<double>(N_POINTS_BE, 0.);
		for (int j = 0; j < N_POINTS_BE; j++) {
			from_chars(v[j].c_str(), v[j].c_str() + line.size(), intermediary[j]);
		}
		// pchip
		pchip tmp(std::move(energy_range()), std::move(intermediary));
		// determine parameters according to energy
		result[i] = tmp(be);
	}
	return result;
}


std::vector<double> RefMapEstimator::load_b(const double wd)
{
	vector<double> result(N_POINTS + 1, 0.);

	string line;
	auto file = ifstream(FILE_B);
	if (!file.is_open()) {
		throw std::exception("Could not load data/b.csv file.");
	}

	for (int i = 0; i < N_POINTS + 1; i++) {
		getline(file, line);
		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		auto intermediary = vector<double>(N_POINTS_WD, 0.);
		for (int j = 0; j < N_POINTS_WD; j++) {
			from_chars(v[j].c_str(), v[j].c_str() + line.size(), intermediary[j]);
		}
		// pchip
		pchip tmp(std::move(wd_range()), std::move(intermediary));
		// determine parameters according to energy
		result[i] = tmp(wd);
	}
	return result;
}


std::vector<double> RefMapEstimator::load_k(const double wd)
{
	vector<double> result(N_POINTS + 1, 0.);

	string line;
	auto file = ifstream(FILE_K);
	if (!file.is_open()) {
		throw std::exception("Could not load data/b.csv file.");
	}

	for (int i = 0; i < N_POINTS + 1; i++) {
		getline(file, line);
		vector<string> v;
		boost::split(v, line, boost::is_any_of(","));
		auto intermediary = vector<double>(N_POINTS_WD, 0.);
		for (int j = 0; j < N_POINTS_WD; j++) {
			from_chars(v[j].c_str(), v[j].c_str() + line.size(), intermediary[j]);
		}
		// pchip
		pchip tmp(std::move(wd_range()), std::move(intermediary));
		// determine parameters according to energy
		result[i] = tmp(wd);
	}
	return result;
}


vector<double> RefMapEstimator::range(double start, double stop, double step)
{
	if ((stop < start && step > 0) || (stop > start && step < 0) || (step == 0)) {
		throw exception("Infinite range");
	}

	int n = lround(ceil((stop - start) / step));
	vector<double> res(n, 0.);
	int j = 0;
	for (double i = start; i < stop; i += step, j++) {
		res[j] = i;
	}
	return res;
}

vector<double> RefMapEstimator::energy_range()
{
	return vector<double>({ 5,10,15,20,25,30 });
}

vector<double> RefMapEstimator::wd_range()
{
	return vector<double>({ 10,12,14,16,18,20 });
}
