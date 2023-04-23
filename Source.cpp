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

using namespace std;

bool inVector(vector<int> v, int i);

int main() {
	GRBEnv* environment = 0;
	environment = new GRBEnv();
	GRBModel model = GRBModel(*environment);
	try {

		std::random_device rd;
		std::mt19937 g(rd());


		int N; // Number of individuals
		int M; // Number of groups
		int T; // Number of roles

		vector<vector<int>> preference; // person i's preference list for people
		vector<int> prefSize; // size of person i's preference for other people
		vector<vector<int>> prefRoles; // R_i
		vector<vector<int>> nonprefRoles; // R_i^N
		vector<vector<int>> indifRoles; // epsilon - R_i U R_i^N
		vector<int> maxGroup; // max members in group j
		vector<int> minGroup; // min members in group j
		vector<int> maxRole; // max members in role r
		vector<int> minRole; // min members in role r
		vector<int> rolesInGroup; // number of roles in group j
		vector<vector<int>> groupRoles; // indeces of roles in group j, takes in [j][arb]
		vector<double> delta; // delta value for each i

		double ALPHA;
		double BETA;
		double GAMMA;
		
		vector<string> id; // Names/ID's of all the individuals in the order they are indexed

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
				yVar[i].push_back(model.addVar(0.0, 1.0, 1.0, GRB_BINARY));
			}
		}

		vector<vector<GRBVar>> zVar;
		for (int i = 0; i < N; i++) {
			zVar.push_back(vector<GRBVar>());
			for (int arb = 0; arb <= prefSize[i]; arb++) {
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
			if (static_cast<int>(indifRoles[i].size()) > 0) {
				for (int arb3 = 0; arb3 < static_cast<int>(indifRoles[i].size()); arb3++) {
					constrbi += wVar[i][indifRoles[i][arb3]];
				}
			}
			objective += ((delta[i]) * ((ALPHA * constrai) + (BETA * constrbi) + (GAMMA * constrci)));
		}
		//set objective

		model.setObjective(objective, GRB_MAXIMIZE);

		//constraints************************************************************************************************************************************

		//max min group size
		for (int j = 0; j < M; j++) {
			GRBLinExpr constrmaxgr = 0;
			for (int i = 0; i < N; i++) {
				constrmaxgr += xVar[i][j];
			}
			model.addConstr(constrmaxgr, GRB_LESS_EQUAL, maxGroup[j]);
		}

		for (int j = 0; j < M; j++) {
			GRBLinExpr constrmingr = 0;
			for (int i = 0; i < N; i++) {
				constrmingr += xVar[i][j];
			}
			model.addConstr(constrmingr, GRB_GREATER_EQUAL, minGroup[j]);
		}

		//i must be in a group ********************************
		for (int i = 0; i < N; i++) {
			GRBLinExpr constringr = 0;
			for (int j = 0; j < T; j++) { // CHANGED FROM XVAR TO WVAR 
				constringr += wVar[i][j]; // CHANGED FROM XVAR TO WVAR
			}
			model.addConstr(constringr, GRB_EQUAL, 1);
		}

		//how many preferred members to match to i
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrprefppl = 0;
			if (prefSize[i] > 0) {
				for (int o = 0; o < prefSize[i]; o++) {
					constrprefppl += zVar[i][o];
				}
			}
			else {
				constrprefppl += zVar[i][0];
			}
			model.addConstr(constrprefppl, GRB_EQUAL, 1);
		}

		//matches students from Pi to i
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrprefpplconnL = 0;
			for (int arb = 0; arb < prefSize[i]; arb++) {
				constrprefpplconnL += yVar[i][arb];
			}
			GRBLinExpr constrprefpplconnR = 0;
			for (int o = 0; o < prefSize[i]; o++) {
				constrprefpplconnR += (o) * (zVar[i][o]);
			}
			model.addConstr(constrprefpplconnL, GRB_EQUAL, constrprefpplconnR);
		}

		//if i matched to k and to j, then k also matched to j
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

		//minimum and maximum of r
		for (int r = 0; r < T; r++) {
			GRBLinExpr constrpplinrl = 0;
			for (int i = 0; i < N; i++) {
				constrpplinrl += wVar[i][r];
			}
			model.addConstr(constrpplinrl, GRB_LESS_EQUAL, maxRole[r]);
		}

		for (int r = 0; r < T; r++) {
			GRBLinExpr constrpplinrl = 0;
			for (int i = 0; i < N; i++) {
				constrpplinrl += wVar[i][r];
			}
			model.addConstr(constrpplinrl, GRB_GREATER_EQUAL, minRole[r]);
		}

		//each i must be in one r
		for (int i = 0; i < N; i++) {
			GRBLinExpr constrinrl = 0;
			for (int j = 0; j < M; j++) {
				constrinrl += xVar[i][j];
			}
			model.addConstr(constrinrl, GRB_EQUAL, 1);
		}

		//each role belongs to exactly one group
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


		model.optimize(); // Magic


		//PRINTING OUT RESULTS - Code below is not necessary for actual use of algorithm, in actual application this is replaced 
		// by code to make the file that is then sent back to the server with all the assignments

		vector<vector<int>> resultV; // resultV[r][arb] means that the arb-th person in role r -> i
		vector<int> resultVrev; // resultVrev[i] is the role that person i is in
		for (int r = 0; r < T; r++) {
			resultV.push_back(vector<int>());
		}
		if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
			cout << "Total objective value: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
			for (int i = 0; i < N; i++) {
				cout << "Person" << i << " assigned to role of index ";
				for (int r = 0; r < T; r++) {
					if (!(wVar[i][r].get(GRB_DoubleAttr_X) == 0)) {
						cout << r << endl;
						resultVrev.push_back(r);
						resultV[r].push_back(i);
					}
				}
			}
		}
		else {
			cout << "No feasible solution" << endl;
		}

		// for making result for groups and then for the number of preferred individuals
		vector<vector<int>> resultGroup;
		vector<int> resultPreferred;
		for (int i = 0; i < M; i++) {
			resultGroup.push_back(vector<int>());
			for (int j = 0; j < static_cast<int>(groupRoles[i].size()); j++) {
				for (int k = 0; k < static_cast<int>(resultV[groupRoles[i][j]].size()); k++) {
					resultGroup[i].push_back(resultV[groupRoles[i][j]][k]);
				}
			}
		}

		for (int i = 0; i < N; i++) {
			resultPreferred.push_back(0);
		}
		for (int j = 0; j < M; j++) {
			for (int arb = 0; arb < static_cast<int>(resultGroup[j].size()); arb++) {
				int preferred = 0;
				for (int arb2 = 0; arb2 < static_cast<int>(resultGroup[j].size()); arb2++) { //static_cast<int>(preference[resultV[r][arb]].size())
					if (inVector(preference[resultGroup[j][arb]], resultGroup[j][arb2])) {
						preferred++;
					}
				}
				resultPreferred[resultGroup[j][arb]] = preferred;
			}
		}
		cout << endl;
		for (int arb = 0; arb < T; arb++) {
			cout << " __________";
		}
		cout << endl << "|";
		int bigstRole = 0;
		for (int r = 0; r < T; r++) {
			cout << "  Role " << r << "  |";
			if (static_cast<int>(resultV[r].size()) > bigstRole) {
				bigstRole = static_cast<int>(resultV[r].size());
			}
		}
		vector<bool> prefRl;
		for (int i = 0; i < N; i++) {
			prefRl.push_back(false);
			for (int arb3 = 0; arb3 < static_cast<int>(prefRoles[i].size()); arb3++) {
				if (prefRoles[i][arb3] == resultVrev[i]) {
					prefRl[i] = true;
				}
			}
		}
		cout << endl << "|";
		for (int arb = 0; arb < bigstRole; arb++) {
			for (int r = 0; r < T; r++) {
				if (static_cast<int>(resultV[r].size()) > arb) {
					if (resultV[r][arb] >= 10) {
						if (prefRl[resultV[r][arb]]) {
							cout << " " << resultPreferred[resultV[r][arb]] << "  " << resultV[r][arb] << "  " << "P" << " |";
						}
						else {
							cout << " " << resultPreferred[resultV[r][arb]] << "  " << resultV[r][arb] << "    |";
						}
					}
					else {
						if (prefRl[resultV[r][arb]]) {
							cout << " " << resultPreferred[resultV[r][arb]] << "  " << resultV[r][arb] << "   " << "P" << " |";
						}
						else {
							cout << " " << resultPreferred[resultV[r][arb]] << "  " << resultV[r][arb] << "     |";
						}
					}
				}
				else {
					cout << "          |";
				}
			}
			cout << endl << "|";
		}
		for (int r = 0; r < T; r++) {
			cout << "__________|";
		}
		cout << endl << "ORIGINAL" << endl;
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	delete environment;
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

