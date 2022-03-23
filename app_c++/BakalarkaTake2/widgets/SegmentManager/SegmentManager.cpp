#include "SegmentManager.h"
#include <QSvgRenderer>

SegmentManager::SegmentManager(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	segmentImage->load(QString(":/gui/svg/bse_detector.svg"));
	segmentImage->renderer()->setAspectRatioMode(Qt::KeepAspectRatio);

	m_labels = { imageLabel0, imageLabel1, imageLabel2, imageLabel3 };
}

SegmentManager::~SegmentManager()
{
}

void SegmentManager::loadImages(std::array<cv::Mat, 4> imgs)
{
	for (int i = 0; i < 4; i++) {
		m_labels[i]->setMat(imgs[i]);
	}
}

void SegmentManager::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		if (m_moving == NOT_MOVING) {
			auto x = event->pos().x();
			auto y = event->pos().y();
			auto xwidg = width();
			auto ywidg = height();
			m_moving = x * 2 / xwidg + y * 2 / ywidg * 2;
		}
	}
}

void SegmentManager::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		if (m_moving != NOT_MOVING) {
			auto x = event->pos().x();
			auto y = event->pos().y();
			auto xwidg = width();
			auto ywidg = height();
			auto target = x * 2 / xwidg + y * 2 / ywidg * 2;

			if (target < 0 || target == m_moving) {
				return;
			}

			swapImages(m_moving, target);
			m_moving = NOT_MOVING;
		}
	}
}

void SegmentManager::swapImages(int first, int second)
{
	assert(first >= 0 && first < 4);
	assert(second >= 0 && second < 4);
	assert(first != second);

	auto tmp = m_labels[first]->fullPixmap();
	m_labels[first]->setPixmap(m_labels[second]->fullPixmap());
	m_labels[second]->setPixmap(tmp);
	emit swapped(first, second);
}
