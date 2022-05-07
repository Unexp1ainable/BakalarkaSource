#include <iostream>
#include "RefMapEstimator.h"
#include <filesystem>
#include <string>

using namespace std;


int main(int argc, char** argv) {
	cout << "Hello world!" << endl;
	try {
		auto est = RefMapEstimator(10., 10., 5.);
		int val = 128;
		cout << "a: " << est.getA(val) << "\n";
		cout << "b: " << est.getB(val) << "\n";
		cout << "k: " << est.getK(val) << "\n";
	}
	catch (std::exception& e) {
		cout << e.what() << "\n";
	}
	return 0;
}