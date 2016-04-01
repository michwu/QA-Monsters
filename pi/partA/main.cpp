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


#define CALLER_STR "Call graph node for function: '"
#define CALLEE_STR "calls function '"

int main(int argc, const char* argv[]) {
  int support = 3;
  int confidence = 65;
  string caller_str = string(CALLER_STR);
  string callee_str = string(CALLEE_STR);

  map<string, set<size_t>*> htMap;                  // a map of function names to sets of integer hashes of functions that are in the function
  map<size_t, string> hashToFuncName;               // maps integer hashes to function names
  set<pair<size_t, size_t> > testPairs;	    				// pairs to test

  if (argc == 3) {
    support = atoi(argv[1]);
    confidence = atoi(argv[2]);
  }

  string curFunc;
  vector<string> prevCallees;
  boost::hash<string> str_hash;
  for (string line; getline(cin, line);) {
    if (line.find(caller_str) != string::npos) {
      set<size_t> *callees = new set<size_t>();
      prevCallees.clear();

      int endIndex = line.find("'", caller_str.length());
      curFunc = line.substr(caller_str.length(), endIndex-caller_str.length());
      htMap[curFunc] = callees;
      hashToFuncName[str_hash(curFunc)] = curFunc;
    } else if (!curFunc.empty() && line.find(callee_str) != string::npos) {
      set<size_t> *callees = htMap[curFunc];

      string callee = line.substr(callee_str.length(), line.length()-callee_str.length()-1);
      size_t calleeHash = str_hash(callee);
      callees->insert(calleeHash);
      hashToFuncName[calleeHash] = callee;
      
      string calleeUpper(boost::to_upper_copy<string>(callee));
      for (vector<string>::iterator it = prevCallees.begin(); it != prevCallees.end(); ++it) {
        string prevCalleeUpper(boost::to_upper_copy<string>(*it));
        if (calleeUpper.compare(prevCalleeUpper) < 0) {
          testPairs.insert(make_pair(str_hash(callee), str_hash(*it)));
        } else {
          testPairs.insert(make_pair(str_hash(*it), str_hash(callee)));
        }
      }

      prevCallees.push_back(callee);
    } else {
      curFunc.clear();
    }
  }

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
