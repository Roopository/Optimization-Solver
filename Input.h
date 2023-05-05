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

class Input {
public: // May change these to private later depending on how this is implemented in the driver program
	Input() {}; // All vectors delete[] themselves and default construct so only default constructor needed

	void readInput(ifstream& fin);

	void readUsername(ifstream& fin);

	void readAllFlights(ifstream& fin);

	void readPPref(ifstream& fin);

	void readFPref(ifstream& fin);

	void finishVectors();

	void printEverything(); // For debugging purposes

	int N = 0; // Number of individuals
	int M = 0; // Number of groups
	int T = 0; // Number of roles

	vector<vector<int>> preference; // person i's preference list for people  DONE
	vector<int> prefSize; // size of person i's preference for other people  NOT DONE
	vector<vector<int>> prefRoles; // R_i  DONE
	vector<vector<int>> nonprefRoles; // R_i^N  EXTERNAL
	vector<vector<int>> indifRoles; // epsilon - R_i U R_i^N  QUESTIONABLE
	vector<int> maxGroup; // max members in group j  TOTAL SET ALL TO 2 
	vector<int> minGroup; // min members in group j  TOTAL SET ALL TO 2
	vector<int> maxRole; // max members in role r   TOTAL  SET ALL TO 1
	vector<int> minRole; // min members in role r   TOTAL  SET ALL TO 1
	vector<int> rolesInGroup; // number of roles in group j   TOTAL SET ALL TO 2
	vector<vector<int>> groupRoles; // indeces of roles in group j, takes in [j][arb]   TOTAL MAKE EACH PAIR INTO A GROUP
	vector<double> delta; // delta value for each i   EXTERNAL

	vector<string> idusername;
};



void Input::readInput(ifstream& fin) {
	string temp;
	fin >> temp;
	fin >> temp;

	if (temp != "\"code\":") {
		cout << "File not oriented correctly" << endl;
		exit(1);
	}
	fin >> temp;
	if (temp != "200,") {
		cout << "Code is not 200" << endl;
		exit(1);
	}


	while (fin >> temp) {
		cout << "Temp: " << temp << endl << endl;
		if (temp == "\"username\":") {
			readUsername(fin);
		}
		else if (temp == "\"colleagues\":") {
			readPPref(fin);
			cout << endl;
		}
		else if (temp == "],") {
			readAllFlights(fin);
		}
		else if (temp == "\"flights\":") {
			cout << "temp" << endl;
			readFPref(fin);
		}
	}
}


void Input::readUsername(ifstream& fin) {
	string temp;
	fin >> temp;
	idusername.push_back(temp.substr(1, temp.size() - 3)); // -3 because first " then last " then , after the "
	N++;
}

void Input::readAllFlights(ifstream& fin) {
	string temp;
	while (temp != "]") {
		if (temp == "\"id\":") {
			M++;
			T += 2;
		}
		fin >> temp;
	}
}

void Input::readPPref(ifstream& fin) {
	string temp;
	fin >> temp;
	if (temp == "[") {
		preference.push_back(vector<int>());
		while (temp != "],") {
			if (temp == "\"id\":") {
				fin >> temp;
				cout << temp.substr(0, temp.length() - 1) << " ";
				preference.back().push_back(stoi(temp.substr(0, temp.length() - 1)));
			}
			fin >> temp;
		}
	}

}

void Input::readFPref(ifstream& fin) {
	string temp;
	fin >> temp;
	cout << "Flights temp: " << temp << endl;

	prefRoles.push_back(vector<int>());
	if (temp == "[") {
		while (temp != "]") {
			fin >> temp;
			if (temp != "]") {
				if (temp[temp.size() - 1] == ',') {
					string temp2 = temp.substr(0, temp.length() - 1);
					prefRoles.back().push_back(stoi(temp2));
				}
				else {
					prefRoles.back().push_back(stoi(temp));
				}
			}
		}
	}
}

void Input::finishVectors() {
	for (int i = 0; i < M; i++) {
		minGroup.push_back(2);
		maxGroup.push_back(2);
		rolesInGroup.push_back(2);
		groupRoles.push_back(vector<int>());
		for (int j = 0; j < rolesInGroup[i]; j++) {
			groupRoles[i].push_back(2 * i + j);
			minRole.push_back(1);
			maxRole.push_back(1);
		}
	}

	delta.resize(N, 0.5); // This needs to be changed so that it takes in the actual delta values from the people

	// Also need NONPREFROLES from the invariants of which planes are illegal for any pilot to fly
  // depending on if algorithm changes may also need INDIFROLES which is just everything that is not in PREF or NOPREF
  
}

void Input::printEverything() {
	for (int i = 0; i < static_cast<int>(idusername.size()); i++) {
		cout << "i = " << i << " : " << idusername[i] << endl;
		cout << idusername[i] << "'s preferences are people: " << endl;
		for (int j = 0; j < static_cast<int>(preference[i].size()); j++) {
			cout << preference[i][j] << " ";
		}
		cout << endl << endl;
	}

	for (int i = 0; i < static_cast<int>(idusername.size()); i++) {
		cout << "i = " << i << " : " << idusername[i] << endl;
		cout << idusername[i] << "'s preferences are flights: " << endl;
		for (int j = 0; j < static_cast<int>(prefRoles[i].size()); j++) {
			cout << prefRoles[i][j] << " ";
		}
		cout << endl << endl;
	}

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < static_cast<int>(groupRoles[i].size()); j++) {
			cout << groupRoles[i][j] << " ";
		}
		cout << endl;
	}

	cout << "There are:\n" << N << " people,\n" << M << " flights,\n" << "and " << T << " roles\n";
}
