#pragma once
#include <Eigen/Dense>

class Facet {
public:
	Facet() = default;
	Facet(double nx, double ny, double nz);

	/**
	 * @brief Returns reference to normal vector
	 * @return normal vector reference
	*/
	Eigen::Vector3d& rn() { return m_n; }


protected:
	Eigen::Vector3d m_n;	// normal vector

	// height of the vertices
	//		0 - 1
	//		|   |
	//		2 - 3
	double m_z0 = 0.f;
	double m_z1 = 0.f;
	double m_z2 = 0.f;
	double m_z3 = 0.f;

};