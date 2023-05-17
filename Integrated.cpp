#include <iostream>
#include <vector>
#include "gurobi_c++.h"
#include <string>
#include <cstdlib>
#include <assert.h>
#include <time.h>
#include <random>
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace std;

bool inVector(vector<int> v, int i);
class Input {
public:
	Input() {};

	void readInput(ifstream& fin);

	void readUsername(ifstream& fin);

	void readAllFlights(ifstream& fin);

	void readPPref(ifstream& fin);

	void readFPref(ifstream& fin);

	void finishVectors();

	void printEverything();



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
	vector<vector<int>> groupRoles; // indeces of roles in group j, takes in [j][arb]  TOTAL MAKE EACH PAIR INTO A GROUP
	vector<double> delta; // delta value for each i   EXTERNAL

	vector<string> idusername;
};

class Output {
public:

	Output() {};

	void outputArray(ofstream& fout, vector<int> resultVrev, int N);
	void outputArray(vector<vector<int>> resultVrev, int N);

};


int main() {
	GRBEnv* environment = 0;
	environment = new GRBEnv();
	GRBModel model = GRBModel(*environment);
	try {

		ifstream fin;

		Input input;

		Output output;

		cout << "WF4 " << "Enter Filename: ";
		string filename;

		cin >> filename;

		cout << endl;

		//char* filename = argv[1];
		fin.open(filename);
		if (!fin.is_open()) {
			cout << "Error opening file" << endl;
			int x = 0;
			cin >> x;
			return(1);
		}

		input.readInput(fin);
		input.finishVectors(); // ALSO NEED NONPREF ROLES FROM THE INVARIANTS OF WHICH PLANES ARE ILLEGAL FOR EACH PILOT TO FLY
		input.printEverything();




		int N = input.N;
		int M = input.M;
		int T = input.T;

		vector<vector<int>> preference;
		vector<int> prefSize;
		vector<vector<int>> prefRoles;
		vector<vector<int>> nonprefRoles;
		vector<int> maxGroup;
		vector<int> minGroup;
		vector<int> maxRole;
		vector<int> minRole;
		vector<int> rolesInGroup;
		vector<vector<int>> groupRoles;
		vector<double> delta;


		preference = input.preference;
		prefSize = input.prefSize;
		prefRoles = input.prefRoles;
		maxGroup = input.maxGroup;
		minGroup = input.minGroup;
		maxRole = input.maxRole;
		minRole = input.minRole;
		rolesInGroup = input.rolesInGroup;
		groupRoles = input.groupRoles;
		delta = input.delta;
		nonprefRoles = input.nonprefRoles;


		const double ALPHA = 0.5;
		const double BETA = 0;
		const double GAMMA = -0.5;

		// code for randomizing preference[] so that there is a uniform distribution of sizes up to N / 3



		vector<string> id;
		//variables**************************************************************************************************************************************

		vector<vector<GRBVar>> xVar;
		for (int i = 0; i < N; i++) {
			xVar.push_back(vector<GRBVar>());
			for (int j = 0; j < M; j++) {
				xVar[i].push_back(model.addVar(0.0, 1.0, 1.0, GRB_BINARY));
			}
		}


		vector<vector<GRBVar>> yVar;
		for (int i = 0; i < N; i++) {
			yVar.push_back(vector<GRBVar>());
			for (int arb = 0; arb < prefSize[i]; arb++) {
				cout << "``" << i << " , " << arb << endl;
				yVar[i].push_back(model.addVar(0.0, 1.0, 1.0, GRB_BINARY));
			}
		}


		vector<vector<GRBVar>> zVar;
		for (int i = 0; i < N; i++) {
			zVar.push_back(vector<GRBVar>());
			for (int arb = 0; arb < 4; arb++) {
				zVar[i].push_back(model.addVar(0.0, 1.0, 1.0, GRB_BINARY));
			}
		}

		vector<vector<GRBVar>> wVar;
		for (int i = 0; i < N; i++) {
			wVar.push_back(vector<GRBVar>());
			for (int r = 0; r < T; r++) {
				wVar[i].push_back(model.addVar(0.0, 1.0, 1.0, GRB_BINARY));
			}
		}


		//objective function*****************************************************************************************************************************
		GRBLinExpr objective = 0;
		for (int i = 0; i < N; i++) {
			for (int o = 0; o < prefSize[i]; o++) {
				objective += ((zVar[i][o]) * (1 - delta[i]) * (o)) / (prefSize[i]);
			}
		}
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrai = 0;
			if (static_cast<int>(prefRoles[i].size()) > 0) {
				for (int arb = 0; arb < static_cast<int>(prefRoles[i].size()); arb++) {
					cout << i << " , " << arb << endl;
					constrai += wVar[i][prefRoles[i][arb]];
				}
			}
			GRBLinExpr constrci = 0;
			if (static_cast<int>(nonprefRoles[i].size()) > 0) {
				for (int arb2 = 0; arb2 < static_cast<int>(nonprefRoles[i].size()); arb2++) {
					constrci += wVar[i][nonprefRoles[i][arb2]];
				}
			}
			GRBLinExpr constrbi = 0;
			objective += ((delta[i]) * ((ALPHA * constrai) + (BETA * constrbi) + (GAMMA * constrci)));
		}
		//set objective

		model.setObjective(objective, GRB_MAXIMIZE);

		//constraints************************************************************************************************************************************

		// (1)
		for (int j = 0; j < M; j++) { // CHANGED TO BE 2 BC ALL FLIGHTS HAVE 2 PILOTS
			GRBLinExpr constrminmaxgr = 0;
			for (int i = 0; i < N; i++) {
				constrminmaxgr += xVar[i][j];
			}
			model.addConstr(constrminmaxgr, GRB_GREATER_EQUAL, 2);
		}

    
		// (2)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constringrL = 0;
			for (int j = 0; j < T; j++) { 
				constringrL += wVar[i][j]; 
			}
			model.addConstr(constringrL, GRB_LESS_EQUAL, 3); // ONLY FOR THIS CASE
		}
    
    
		// (3)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constringrG = 0;
			for (int j = 0; j < T; j++) { // CHANGED FROM XVAR TO WVAR 
				constringrG += wVar[i][j]; // CHANGED FROM XVAR TO WVAR
			}
			model.addConstr(constringrG, GRB_GREATER_EQUAL, 2); // ONLY FOR THIS CASE
		}

    
		// (4)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrprefppl = 0;
			if (prefSize[i] > 0) {
				for (int o = 0; o < 4; o++) { // CHANGED TO 4 (1+MAXIMUM NUMBER OF GROUPS POSSIBLE) NEED TO AUTOMATE THIS
					constrprefppl += zVar[i][o];
				}
			}
			else {
				constrprefppl += zVar[i][0];
			}
			model.addConstr(constrprefppl, GRB_EQUAL, 1);
		}

    
		// (5)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrprefpplconnL = 0;
			for (int arb = 0; arb < prefSize[i]; arb++) {
				constrprefpplconnL += yVar[i][arb];
			}
			GRBLinExpr constrprefpplconnR = 0;
			for (int o = 0; o < 4; o++) { // ALSO CHANGED TO 4 SAME AS ABOVE
				constrprefpplconnR += (o) * (zVar[i][o]);
			}
			model.addConstr(constrprefpplconnL, GRB_EQUAL, constrprefpplconnR);
		}
    
    
		// (6)
		for (int r = 0; r < T; r++) {
			GRBLinExpr constrpplinrl = 0;
			for (int i = 0; i < N; i++) {
				constrpplinrl += wVar[i][r];
			}
			model.addConstr(constrpplinrl, GRB_EQUAL, 1);
		}

    
		// (7)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrinrlL = 0;
			for (int j = 0; j < M; j++) {
				constrinrlL += xVar[i][j];
			}
			model.addConstr(constrinrlL, GRB_LESS_EQUAL, 3); // CHANGED TO 2 VERSIONS (GREATER THAN)
		}

    
		// (8)
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrinrlG = 0;
			for (int j = 0; j < M; j++) {
				constrinrlG += xVar[i][j];
			}
			model.addConstr(constrinrlG, GRB_GREATER_EQUAL, 2); // CHANGED TO 2 VERSIONS (LESS THAN)
		}
    
    
		// (9)
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				for (int arb = 0; arb < prefSize[i]; arb++) {
					GRBLinExpr constrpplgrconnL = 0;
					constrpplgrconnL += yVar[i][arb];
					constrpplgrconnL += xVar[i][j];
					constrpplgrconnL -= 1;
					GRBLinExpr constrpplgrconnR = 0;
					constrpplgrconnR += xVar[preference[i][arb]][j];
					model.addConstr(constrpplgrconnL, GRB_LESS_EQUAL, constrpplgrconnR);
				}
			}
		}
    
    
		// (10)
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				GRBLinExpr constrrlgrconnL = 0;
				for (int arb = 0; arb < rolesInGroup[j]; arb++) {
					constrrlgrconnL += wVar[i][groupRoles[j][arb]];
				}
				GRBLinExpr constrrlgrconnR = 0;
				constrrlgrconnR += xVar[i][j];
				model.addConstr(constrrlgrconnL, GRB_LESS_EQUAL, constrrlgrconnR);
			}
		}


		model.optimize();

    
		vector<vector<int>> resultV; // resultV[r][arb] means that the arb-th person in role r -> i
		vector<vector<int>> resultVrev; // resultVrev[i] is the role that person i is in
		for (int r = 0; r < T; r++) {
			resultV.push_back(vector<int>());
		}
		for (int i = 0; i < N; i++) {
			resultVrev.push_back(vector<int>());
		}
		if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
			cout << "Total objective value: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
			for (int i = 0; i < N; i++) {
				cout << "Person " << i << " assigned to role of index ";
				for (int r = 0; r < T; r++) {
					if (!(wVar[i][r].get(GRB_DoubleAttr_X) == 0)) {
						cout << r << " ";
						resultVrev[i].push_back(r);
						resultV[r].push_back(i);
					}
				}
				cout << endl;
			}
		}

		output.outputArray(resultVrev, N);
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	delete environment;
	int x;
	cin >> x;
	return 0;
}

bool inVector(vector<int> v, int i) {
	for (int j = 0; j < (static_cast<int>(v.size())); j++) {
		if (v[j] == i) {
			return true;
		}
	}
	return false;
}



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

void Output::outputArray(vector<vector<int>> resultVrev, int N) {
	cout << "data = {'matchs': [" << endl << "    ";
	int count = 0;
	for (int i = 0; i < N; i++) {

		for (int j = 0; j < static_cast<int>(resultVrev[i].size()); j++) {
			count++;

			cout << "    {'pilot_id': " << i << ", 'role_id': " << resultVrev[i][j] % 2 << ", 'flight_id': " << resultVrev[i][j] / 2 << "}"; // ONLY WORKS BECAUSE THE FLIGHTS ALL HAVE 2 ROLES

				if (i != N - 1) {
					cout << ",";
				}
				else {
					if (j == static_cast<int>(resultVrev[i].size()) - 1) {
						cout << "]";
					}
					else {
						cout << ",";
					}
				}

			if ((count % 2 == 0) && (i != N - 1)) {
				cout << endl << "    ";
			}
		}
	}

	cout << endl << "}" << endl;
}





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

	prefRoles.push_back(vector<int>());
	if (temp == "[") {
		while (temp != "]") {
			fin >> temp;
			if (temp != "]") {
				if (temp[temp.size() - 1] == ',') {
					string temp2 = temp.substr(0, temp.length() - 1);
					int temp2i = stoi(temp2);
					prefRoles.back().push_back(2 * temp2i);
					prefRoles.back().push_back(2 * temp2i + 1);
				}
				else {
					int tem2i = stoi(temp);
					prefRoles.back().push_back(2 * tem2i);
					prefRoles.back().push_back(2 * tem2i + 1);
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

	for (int i = 0; i < N; i++) {
		prefSize.push_back(static_cast<int>(preference[i].size()));
	}

	for (int i = 0; i < N; i++) {
		nonprefRoles.push_back(vector<int>());
		indifRoles.push_back(vector<int>());
		for (int j = 0; j < T; j++) {
			if (!inVector(prefRoles[i], j)) {
				indifRoles[i].push_back(j);
			}
		}
	}

	delta.resize(N, 0.5); // This needs to be changed so that it takes in the actual delta values from the people



	// ALSO NEED NONPREF ROLES FROM THE INVARIANTS OF WHICH PLANES ARE ILLEGAL FOR EACH PILOT TO FLY
}

void Input::printEverything() {
	for (int i = 0; i < static_cast<int>(idusername.size()); i++) {
		cout << "i = " << i << " : " << idusername[i] << endl;
		cout << idusername[i] << "'s preferences are " << prefSize[i] << " people: " << endl;
		for (int j = 0; j < static_cast<int>(preference[i].size()); j++) {
			cout << preference[i][j] << " ";
		}
		cout << endl << endl;
	}

	for (int i = 0; i < static_cast<int>(idusername.size()); i++) {
		cout << "i = " << i << " : " << idusername[i] << endl;
		cout << idusername[i] << "'s preferences are roles: " << endl;
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

	for (int i = 0; i < N; i++) {
		cout << "PREFERRED ROLES OF " << i << ":" << endl;
		for (int j = 0; j < static_cast<int>(prefRoles[i].size()); j++) {
			cout << prefRoles[i][j] << " ";
		}

		cout << endl << "INDIFFERENT ROLES OF " << i << ":" << endl;
		for (int k = 0; k < static_cast<int>(indifRoles[i].size()); k++) {
			cout << indifRoles[i][k] << " ";
		}

		cout << endl << endl;
	}

	cout << "There are:\n" << N << " people,\n" << M << " flights,\n" << "and " << T << " roles\n";
}
