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

	void outputArray(ofstream& fout, vector<vector<int>> resultVrev, int N); // For when outputting to an outside file
	void outputArray(vector<vector<int>> resultVrev, int N); // For when outputting to the console

};



void Output::outputArray(ofstream& fout, vector<vector<int>> resultVrev, int N) {
	fout << "data = {'matchs': [" << endl << "    ";
	int count = 0;
	for (int i = 0; i < N; i++) {

		for (int j = 0; j < static_cast<int>(resultVrev[i].size()); j++) {
			count++;

			fout << "    {'pilot_id': " << i << ", 'role_id': " << resultVrev[i][j] % 2 << ", 'flight_id': " << resultVrev[i][j] / 2 << "}"; // ONLY WORKS BECAUSE THE FLIGHTS ALL HAVE 2 ROLES

			if (i != N - 1) {
				fout << ",";
				if ((count % 2 == 0)) {
					fout << endl << "    ";
				}
			}
			else {
				if (j == static_cast<int>(resultVrev[i].size()) - 1) {
					fout << "]";
				}
				else {
					fout << ",";
					if ((count % 2 == 0)) {
						fout << endl << "    ";
					}
				}
			}
		}
	}
	
	fout << endl << "}" << endl;
}

void Output::outputArray(vector<vector<int>> resultVrev, int N) {
	cout << "data = {'matchs': [" << endl << "    ";
	int count = 0;
	for (int i = 0; i < N; i++) {

		for (int j = 0; j < static_cast<int>(resultVrev[i].size()); j++) {
			count++;

			cout << "    {'pilot_id': " << i << ", 'role_id': " << resultVrev[i][j] % 2 << ", 'flight_id': " << resultVrev[i][j] / 2 << "}"; // ONLY WORKS BECAUSE THE FLIGHTS ALL HAVE 2 ROLES
			
			if (i != N - 1) {
				cout << ",";
				if ((count % 2 == 0)) {
					cout << endl << "    ";
				}
			}
			else {
				if (j == static_cast<int>(resultVrev[i].size()) - 1) {
					cout << "]";
				}
				else {
					cout << ",";
					if ((count % 2 == 0)) {
						cout << endl << "    ";
					}
				}
			}
		}
	}

	cout << endl << "}" << endl;
}
