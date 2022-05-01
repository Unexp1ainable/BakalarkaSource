#include "Interpolator.h"
#include "InterpolatorData.h"

Interpolator::Interpolator() : 
	m_a(std::move(getValueData()), std::move(getAData())), 
	m_b(std::move(getValueData()), std::move(getBData())), 
	m_k(std::move(getValueData()), std::move(getKData())) {
}