#include "FacetMatrix.h"


using namespace std;

FacetMatrix::FacetMatrix(unsigned int height, unsigned int width) : m_cells(height, vector<Facet>(width)) {
}