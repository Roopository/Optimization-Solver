#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <vector>
#include <unordered_map>

using namespace std;


class Output {
public:

	Output() {}; // No ADT's required so no need for anything besides the default constructor

	void outputArray(ofstream& fout, vector<int> resultVrev, int N); // For when outputting to an outside file
	void outputArray(vector<int> resultVrev, int N); // For when outputting to the console

};



void Output::outputArray(ofstream& fout, vector<int> resultVrev, int N) {
	fout << "data = {'matches': [" << endl;
	for (int i = 0; i < N; i++) {

		fout << "\t{'pilot_id': " << i << ", 'role_id': " << resultVrev[i] << ", 'flight_id': " << resultVrev[i] / 2 << "}";

		if (i != N - 1) {
			fout << ",";
		}
		else {
			fout << "]";
		}

		if (i % 2 == 1) {
			fout << endl;
		}

	}

	fout << "}" << endl;
}

void Output::outputArray(vector<int> resultVrev, int N) {
	cout << "data = {'matches': [" << endl;
	for (int i = 0; i < N; i++) {

		cout << "\t{'pilot_id': " << i << ", 'role_id': " << resultVrev[i] << ", 'flight_id': " << resultVrev[i] / 2 << "}";

		if (i != N - 1) {
			cout << ",";
		}
		else {
			cout << "]";
		}

		if (i % 2 == 1) {
			cout << endl;
		}

	}

	cout << "}" << endl;
}
