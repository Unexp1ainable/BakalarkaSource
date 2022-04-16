#include "Superellipse.h"

#include <algorithm>

using namespace std;

Superellipse::Superellipse(double a, double b, double c, double h, double k) : m_a(a), m_b(b), m_c(c), m_h(h), m_k(k)
{
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
	int hx = lround(hi / 2);

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
	double x = -m_h;
	double y = -m_k;
	vector<QPointF> result{ {x,y} };

	// top right
	while (true)
	{
		x++;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x--;
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
			if (abs(y - ny) > 1) {
				x--;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}

	// right
	while (true)
	{
		y--;
		auto res = rootsX(y);
		double nx = 0.;
		if (res.n == 0) {
			y++;
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
			if (abs(x - nx) > 1) {
				y++;
				break;
			}
			x = nx;
			result.push_back(QPointF(nx, y));
		}
	}

	// down
	while (true)
	{
		x--;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x++;
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
			if (abs(y - ny) > 1) {
				x++;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}

	// left
	while (true)
	{
		y++;
		auto res = rootsX(y);
		double nx = 0.;
		if (res.n == 0) {
			y--;
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
			if (abs(x - nx) > 1) {
				y--;
				break;
			}
			x = nx;
			result.push_back(QPointF(nx, y));
		}
	}

	// top left
	while (x < -m_h)
	{
		x++;
		auto res = rootsY(x);
		double ny = 0.;
		if (res.n == 0) {
			x--;
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
			if (abs(y - ny) > 1) {
				x--;
				break;
			}
			y = ny;
			result.push_back(QPointF(x, ny));
		}
	}
	return result;
}

QPointF Superellipse::closestMidpoint(const Superellipse& el2) const
{
	return QPointF();
}

vector<QPointF> Superellipse::intersections(const Superellipse& el2) const
{
	double lastDiff = 0.;
	double lastX = 0.;
	double lastY = 0.;
	bool expectIntersection = false;
	bool recovering = false;
	vector<QPointF> results{};

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

		if (diff < 1 && !recovering) {
			expectIntersection = true;
			lastDiff = diff;
			lastX = x;
			lastY = y;
		}
		if (diff > 1) {
			recovering = false;
		}
	}

	return results;
}

vector<QPointF> Superellipse::findPOIs(const Superellipse& el2) const
{
	auto inters = intersections(el2);
	if (inters.size() == 0) {
		return vector<QPointF>{ {closestMidpoint(el2)} };
	}
	return inters;
}

void Superellipse::calculateBoundingBox()
{
	auto p1 = QPointF(-m_h - m_a, -m_k);
	auto p2 = QPointF(-m_h + m_a, -m_k - 2 * m_b);
	m_bBox = make_pair(p1, p2);
}
