#include "Superellipse.h"

#include <algorithm>

using namespace std;

Superellipse::Superellipse(double a, double b, double c, double h, double k) : m_a(a), m_b(b), m_c(c), m_h(h), m_k(k)
{
	calculateBoundingBox();
}

void Superellipse::changeSegment(Segments seg, int width, int height)
{
	if (seg == Segments::Q1) {
		return;
	}

	double olda = m_a;
	double oldb = m_b;
	double oldh = m_h;
	double oldk = m_k;

	if (seg == Segments::Q2) {
		m_a = oldb;
		m_b = olda;
		m_h = -oldk - oldb + width / 2.;
		m_k = -oldh - olda + height / 2.;
	}
	else if (seg == Segments::Q3) {
		m_k = height - oldk - 2 * oldb;
	}
	else if (seg == Segments::Q4) {
		m_a = oldb;
		m_b = olda;
		m_h = -width / 2. + oldk + oldb;
		m_k = +oldh - olda + height / 2.;
	}

	calculateBoundingBox();
}

void Superellipse::scale(int width, int height)
{
	m_a *= width;
	m_b *= height;
	m_h *= width;
	m_k *= height;
	calculateBoundingBox();
}

RootResult Superellipse::rootsX(double y) const
{
	auto mid = 1 - pow(abs((y + m_k + m_b) / m_b), (m_c));
	if (mid < 0) {
		return RootResult(0, 0., 0.);
	}
	if (mid == 0) {
		return RootResult(1, -m_h, 0.);
	}
	auto sD = m_a * pow(mid, (1 / m_c));
	return RootResult(2, (sD - m_h), (-sD - m_h));
}

RootResult Superellipse::rootsY(double x) const
{
	auto mid = 1 - pow(abs((x + m_h) / m_a), (m_c));
	if (mid < 0) {
		return RootResult(0, 0., 0.);
	}
	if (mid == 0) {
		return RootResult(1, -m_b - m_k, 0.);
	}
	auto sD = m_b * pow(mid, (1 / m_c));
	return RootResult(2, (sD - m_b - m_k), (-sD - m_b - m_k));
}

QPointF Superellipse::autoRoots(double x, double y, bool& ok) const
{
	auto dx1 = x - m_bBox.first.x();
	auto dx2 = m_bBox.second.x() - x;
	auto dx = dx1 < dx2 ? dx1 : dx2;

	auto dy1 = y - m_bBox.second.y();
	auto dy2 = m_bBox.first.y() - y;
	auto dy = dy1 < dy2 ? dy1 : dy2;

	ok = true;

	if (dx < dy) {
		auto res = rootsX(y);
		if (res.n == 0) {
			ok = false;
			return QPointF();
		}
		if (res.n == 1) {
			return QPointF(res.r1, y);
		}
		else {
			if (abs(res.r1 - x) < abs(res.r2 - x)) {
				return QPointF(res.r1, y);
			}
			else {
				return QPointF(res.r2, y);
			}
		}
	}
	else {
		auto res = rootsY(x);
		if (res.n == 0) {
			ok = false;
			return QPointF();
		}
		if (res.n == 1) {
			return QPointF(x, res.r1);
		}
		else {
			if (abs(res.r1 - y) < abs(res.r2 - y)) {
				return QPointF(x, res.r1);
			}
			else {
				return QPointF(x, res.r2);
			}
		}
	}
}

vector<QPoint> Superellipse::rasterize() const
{
	if (m_a == 0 || m_b == 0) {
		return vector<QPoint>{};
	}

	vector<QPoint> result{};
	auto ystart = -lround(m_k);
	auto yend = lround(m_b * 2) - ystart;
	auto xstart = lround(-m_a - m_h);
	auto xend = lround(m_a - m_h);

	for (int y = ystart; y > -yend; y--) {
		RootResult res = rootsX(y);
		if (res.n == 0) {
			continue;
		}
		result.push_back(QPoint(lround(res.r1), y));
		if (res.n == 1) {
			continue;
		}
		result.push_back(QPoint(lround(res.r2), y));
	}

	for (int x = xstart; x < xend; x++) {
		RootResult res = rootsY(x);
		if (res.n == 0) {
			continue;
		}
		result.push_back(QPoint(x, lround(res.r1)));
		if (res.n == 1) {
			continue;
		}
		result.push_back(QPoint(x, lround(res.r2)));
	}

	// delete duplicates
	sort(result.begin(), result.end(), [](QPoint a, QPoint b) {
		if (a.x() == b.x()) {
			return a.y() < b.y();
		}
		return a.y() < b.y(); });
	result.erase(unique(result.begin(), result.end()), result.end());
	return result;
}

void Superellipse::draw(cv::Mat& img) const
{
	auto points = rasterize();
	int wi = img.cols;
	int hi = img.rows;
	int hx = lround(hi / 2.);

	for (auto [x, y] : points) {
		x += hx;
		y = -y;
		if (0 <= x && x < wi && 0 <= y && y < hi) {
			img.at<uint8_t>(y, x) = 255;
		}
	}
}

vector<QPointF> Superellipse::borderPoints() const
{
	double xstep = abs(m_bBox.second.x() - m_bBox.first.x())/250.;
	double ystep = abs(m_bBox.second.y() - m_bBox.first.y())/250.;
	double step = xstep < ystep ? xstep : ystep;
	double x = -m_h;
	double y = -m_k;
	vector<QPointF> result{ {x,y} };

	// top right
	while (true)
	{
		x+=step;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x-=step;
			break;
		}
		if (res.n == 1) {
			ny = res.r1;
		}
		else {
			if (abs(res.r1 - y) < abs(res.r2 - y)) {
				ny = res.r1;
			}
			else {
				ny = res.r2;
			}
			if (abs(y - ny) > step) {
				x-=step;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}

	// right
	while (true)
	{
		y-=step;
		auto res = rootsX(y);
		double nx = 0.;
		if (res.n == 0) {
			y+=step;
			break;
		}
		if (res.n == 1) {
			nx = res.r1;
		}
		else {
			if (abs(res.r1 - x) < abs(res.r2 - x)) {
				nx = res.r1;
			}
			else {
				nx = res.r2;
			}
			if (abs(x - nx) > step) {
				y+=step;
				break;
			}
			x = nx;
			result.push_back(QPointF(nx, y));
		}
	}

	// down
	while (true)
	{
		x-=step;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x+=step;
			break;
		}
		if (res.n == 1) {
			ny = res.r1;
		}
		else {
			if (abs(res.r1 - y) < abs(res.r2 - y)) {
				ny = res.r1;
			}
			else {
				ny = res.r2;
			}
			if (abs(y - ny) > step) {
				x+=step;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}

	// left
	while (true)
	{
		y+=step;
		auto res = rootsX(y);
		double nx = 0.;
		if (res.n == 0) {
			y-=step;
			break;
		}
		if (res.n == 1) {
			nx = res.r1;
		}
		else {
			if (abs(res.r1 - x) < abs(res.r2 - x)) {
				nx = res.r1;
			}
			else {
				nx = res.r2;
			}
			if (abs(x - nx) > step) {
				y-=step;
				break;
			}
			x = nx;
			result.push_back(QPointF(nx, y));
		}
	}

	// top left
	while (x < -m_h)
	{
		x+=step;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x-=step;
			break;
		}
		if (res.n == 1) {
			ny = res.r1;
		}
		else {
			if (abs(res.r1 - y) < abs(res.r2 - y)) {
				ny = res.r1;
			}
			else {
				ny = res.r2;
			}
			if (abs(y - ny) > step) {
				x-=step;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}
	return result;
}

vector<QPointF> Superellipse::findPOIs(const Superellipse& el2) const
{
	vector<QPointF> results{};
	if (isNull() || el2.isNull())
		return results;

	static constexpr int const NONE = 9999999;
	double globalMin = NONE;
	QPointF gmpt;

	double xstep = abs(m_bBox.second.x() - m_bBox.first.x()) / 250.;
	double ystep = abs(m_bBox.second.y() - m_bBox.first.y()) / 250.;
	double step = xstep < ystep ? xstep : ystep;

	double lastDiff = 0.;
	double lastX = 0.;
	double lastY = 0.;
	bool expectIntersection = false;
	bool recovering = false;

	bool ok;

	for (const auto& [x, y] : borderPoints()) {
		auto pt = el2.autoRoots(x, y, ok);
		if (!ok) {
			continue;
		}
		double diff = abs(y - pt.y() + x - pt.x());
		if (expectIntersection) {
			if (lastDiff < diff) {
				results.push_back(QPointF(lastX, lastY));
				expectIntersection = false;
				recovering = true;
				continue;
			}
			else {
				lastDiff = diff;
				lastX = x;
				lastY = y;
				continue;
			}
		}

		if (diff < step*2 && !recovering) {
			expectIntersection = true;
			lastDiff = diff;
			lastX = x;
			lastY = y;
		}
		if (diff > step) {
			recovering = false;
		}

		// look for closest points
		if (diff < globalMin) {
			gmpt = QPointF((pt.x() + x) / 2., (pt.y() + y)/2);
			globalMin = diff;
		}
	}

	if (results.size() == 0) {
		if (globalMin != NONE) {
			results.push_back(gmpt);
		}
		// if no intersections and no closest points were found, use average of midpoints
		else {
			double ca1 = m_a / (m_a + el2.m_a);
			double ca2 = el2.m_a / (m_a + el2.m_a);
			double cb1 = m_b / (m_b + el2.m_b);
			double cb2 = el2.m_b / (m_b + el2.m_b);
			results.push_back(QPointF(-(m_h * ca2 + el2.m_h * ca1), -((m_k + m_b)*cb2 + (el2.m_k + el2.m_b) * cb1)));
		}
	}

	return results;
}

bool Superellipse::isNull() const 
{
	return m_bBox.first.x() == m_bBox.second.x() || m_bBox.first.y() == m_bBox.second.y();
}

void Superellipse::calculateBoundingBox()
{
	auto p1 = QPointF(-m_h - m_a, -m_k);
	auto p2 = QPointF(-m_h + m_a, -m_k - 2 * m_b);
	m_bBox = make_pair(p1, p2);
}
