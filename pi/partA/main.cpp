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
#define EXT_CALLEE_STR "calls external node"

int main(int argc, const char* argv[]) {
  int support = 3;
  int confidence = 65;
  string caller_str = string(CALLER_STR);
  string callee_str = string(CALLEE_STR);
  string ext_callee_str = string(EXT_CALLEE_STR);

  map<string, set<size_t>*> htMap;                  // a map of function names to sets of integer hashes of functions that are in the function
  map<size_t, string> hashToFuncName;               // maps integer hashes to function names
  set<pair<size_t, size_t> > testPairs;	    				// pairs to test

  if (argc == 3) {
    support = atoi(argv[1]);
    confidence = atoi(argv[2]);
  }

  string curFunc;
  set<string> prevCallees;
  boost::hash<string> str_hash;
  for (string line; getline(cin, line);) {
    if (line.find(caller_str) != string::npos) {
      set<size_t> *callees = new set<size_t>();
      prevCallees.clear();

      int startIndex = line.find("'");
      int endIndex = line.find("'", startIndex+1);
      curFunc = line.substr(startIndex+1, endIndex-startIndex-1);
      htMap[curFunc] = callees;
      hashToFuncName[str_hash(curFunc)] = curFunc;
    } else if (!curFunc.empty() && line.find(callee_str) != string::npos) {
      set<size_t> *callees = htMap[curFunc];

      int startIndex = line.find("'");
      int endIndex = line.find("'", startIndex+1);
      string callee = line.substr(startIndex+1, endIndex-startIndex-1);

      if (prevCallees.find(callee) != prevCallees.end()) {
        continue;
      }

      size_t calleeHash = str_hash(callee);
      callees->insert(calleeHash);
      hashToFuncName[calleeHash] = callee;

      string calleeUpper(boost::to_upper_copy<string>(callee));
      for (set<string>::iterator it = prevCallees.begin(); it != prevCallees.end(); ++it) {
        string prevCalleeUpper(boost::to_upper_copy<string>(*it));
        if (calleeUpper.compare(prevCalleeUpper) < 0) {
          testPairs.insert(make_pair(str_hash(callee), str_hash(*it)));
        } else {
          testPairs.insert(make_pair(str_hash(*it), str_hash(callee)));
        }
      }

      prevCallees.insert(callee);
    } else if (line.find(ext_callee_str) == string::npos) {
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

    float confidenceA = (((float)supportAB)/(supportA)) * 100.0;
    if(supportAB >= support && confidenceA >= (float)confidence) {
      for (vector<string>::iterator listIter = aOnlyList.begin(); listIter != aOnlyList.end(); ++listIter) {
				string funcName = *listIter;
        printf("bug: %s in %s, pair: (%s, %s), support: %d, confidence: %.2f\%\n",
              hashToFuncName[a].c_str(),
              funcName.c_str(),
              hashToFuncName[a].c_str(),
              hashToFuncName[b].c_str(),
              supportAB,
              confidenceA);
      }
    }

    float confidenceB = (((float)supportAB)/(supportB)) * 100.0;
    if(supportAB >= support && confidenceB >= (float)confidence) {
      for (vector<string>::iterator listIter = bOnlyList.begin(); listIter != bOnlyList.end(); ++listIter) {
				string funcName = *listIter;
        printf("bug: %s in %s, pair: (%s, %s), support: %d, confidence: %.2f\%\n",
              hashToFuncName[b].c_str(),
              funcName.c_str(),
              hashToFuncName[a].c_str(),
              hashToFuncName[b].c_str(),
              supportAB,
              confidenceB);
      }
    }
  }

  // cleanup
  for (map<string, set<size_t>*>::iterator iter = htMap.begin(); iter != htMap.end(); ++iter) {
    delete iter->second;
  }
}
