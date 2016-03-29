#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>


using namespace std;

string CALLER_STR = "Call graph node for function: '";
string CALLEE_STR = "calls function '";

int main(int argc, const char* argv[]) {
  int support = 3;
  int confidence = 65;

  map<string, set<size_t>*> htMap;							  // a map of function names to sets of integer hashes of functions that are in the function
  map<size_t, string> hashToFuncName;							// maps integer hashes to function names
  set<pair<size_t, size_t> >	testPairs;					// pairs to test

  if (argc == 3) {
    support = atoi(argv[1]);
    confidence = atoi(argv[2]);
  }

  string curFunc;
  boost::hash<std::string> str_hash;
  for (string line; getline(cin, line);) {
    if (line.find(CALLER_STR) != string::npos) {
      set<size_t> *callees = new set<size_t>();

      int endIndex = line.find("'", CALLER_STR.length());
      curFunc = line.substr(CALLER_STR.length(), endIndex-CALLER_STR.length());
      htMap[curFunc] = callees;
      hashToFuncName[str_hash(curFunc)] = curFunc;
    }

		if (line.find(CALLEE_STR) != string::npos) {
      std::set<size_t> *callees = htMap[curFunc];

      string callee = line.substr(CALLEE_STR.length(), line.length()-CALLEE_STR.length());
      size_t calleeHash = str_hash(callee);
      callees->insert(calleeHash);
      hashToFuncName[calleeHash] = callee;

      std::string curFuncUpper(boost::to_upper_copy<std::string>(curFunc));
      std::string calleeUpper(boost::to_upper_copy<std::string>(callee));
      if (curFuncUpper.compare(calleeUpper)) {
        testPairs.insert(make_pair(str_hash(curFuncUpper), str_hash(calleeUpper)));
      } else {
				testPairs.insert(make_pair(str_hash(calleeUpper), str_hash(curFuncUpper)));
      }
    }
  }

  // assume above is done
  for (set<pair<size_t, size_t> >::iterator it = testPairs.begin(); it != testPairs.end(); ++it) {
    size_t a = it->first;
    size_t b = it->second;
    int supportAB = 0, supportA = 0, supportB = 0;
    vector<string> aOnlyList;	// list of functions that only call A
    vector<string> bOnlyList;	// list of functions that only call B

    for (map<string, set<size_t>*>::iterator htMapIt = htMap.begin(); htMapIt != htMap.end(); ++htMapIt) {
      string funcName = htMapIt->first;
      set<size_t> *functionSet = htMapIt->second;

      bool aFound = (functionSet->find(a) != functionSet->end());
      bool bFound = (functionSet->find(b) != functionSet->end());

      if (aFound && bFound) {
        supportAB++;
        supportA++;
        supportB++;
      } else if (aFound) {
        supportA++;
        aOnlyList.push_back(funcName);
      } else if (bFound) {
        supportB++;
        bOnlyList.push_back(funcName);
      }
    }

    double confidenceA = (((double)supportAB)/((double)supportA)) * 100.0;
    if(supportAB >= support && confidenceA >= (double)confidence) {
      for (vector<string>::iterator listIter = aOnlyList.begin(); listIter != aOnlyList.end(); ++listIter) {
				string funcName = *listIter;
    		cout << "bug: " << hashToFuncName[a] << "A in " << funcName << ", pair: (" << hashToFuncName[a] << ", " << hashToFuncName[b] << "), support: " << supportAB << ", confidence: " << setprecision(9) << confidenceA << "%" << endl;
      }
    }

    double confidenceB = (((double)supportAB)/((double)supportB)) * 100.0;
    if(supportAB >= support && confidenceB >= (double)confidence) {
      for (vector<string>::iterator listIter = bOnlyList.begin(); listIter != bOnlyList.end(); ++listIter) {
				string funcName = *listIter;
    		cout << "bug: " << hashToFuncName[b] << "A in " << funcName << ", pair: (" << hashToFuncName[a] << ", " << hashToFuncName[b] << "), support: " << supportAB << ", confidence: " << setprecision(9) << confidenceB << "%" << endl;
      }
    }
  }

  // cleanup
  for (map<string, set<size_t>*>::iterator iter = htMap.begin(); iter != htMap.end(); ++iter) {
    delete iter->second;
  }
}
