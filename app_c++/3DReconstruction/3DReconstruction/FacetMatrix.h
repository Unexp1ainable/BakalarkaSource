#pragma once
#include "Facet.h"
#include <vector>

class FacetMatrix {
public:
	FacetMatrix() = default;
	FacetMatrix(unsigned int height, unsigned int width);

	unsigned int height() { return m_height; }
	unsigned int width() { return m_width; }

	bool isNull() { return m_height == 0 || m_width == 0; }

	std::vector<Facet>& operator[](size_t i) { return m_cells[i]; }
protected:
	std::vector<std::vector<Facet>> m_cells;

	unsigned int m_height = 0;
	unsigned int m_width = 0;

};