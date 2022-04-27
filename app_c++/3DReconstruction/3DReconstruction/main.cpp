#include <Eigen/Dense>
#include <Eigen/IterativeLinearSolvers>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include "Facet.h"
#include "FacetMatrix.h"

using namespace std;


FacetMatrix loadFile(string path) {
	auto file = ifstream(path);
	if (!file.is_open()) {
		return FacetMatrix();
	}
	long height, width;
	file >> height >> width;
	if (height < 0 || width < 0) {
		return FacetMatrix();
	}

	auto result = FacetMatrix(height, width);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			file >> result[y][x].rn()(0) >> result[y][x].rn()(1) >> result[y][x].rn()(2);
		}
	}
	
	return result;
}



int main()
{
	string path;
	cout << "Please specify path to the normal file:\n";
	getline(cin, path);
	path = "C:/Users/samor/Desktop/VUT/5_semester/Bakalarka/source/app_c++/sfg/DGP/release/Data/test.txt";

	auto facetMat = loadFile(path);
	if (facetMat.isNull()) {
		cerr << "Invalid file.";
		return 1;
	}
	
	Eigen::MatrixXd A;
	Eigen::VectorXd b;
	// fill A
	// fill b (local shaping)
	// throw it at solver
	Eigen::LeastSquaresConjugateGradient<Eigen::MatrixXd> solver;
	solver.compute(A);
	auto x = solver.solve(b);
	return 0;
}