#include <iostream>
#include "RefMapEstimator.h"
#include <filesystem>
#include <string>

using namespace std;


int main(int argc, char** argv) {
	cout << "Hello world!" << endl;
	try {
		for (int a : {5, 10, 15, 20, 25, 30})
		{
			cout << a << "\n";
			auto est = RefMapEstimator(a, 10., 5.);
		}
	}
	catch (std::exception& e) {
		cout << e.what() << "\n";
	}
	return 0;
}