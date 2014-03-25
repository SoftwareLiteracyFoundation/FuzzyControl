using namespace std;
#include <iostream>
#include "FuzzyControl.h"

#ifdef DEBUG
//---------------------------------------------------------------
// DebugMsg
//---------------------------------------------------------------
void  DebugMsg(const char *msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(const char *msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(const char *msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(const char *msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(string msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(string msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(string msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugMsg(string msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}
#endif

#ifdef DEBUG_ALL
//---------------------------------------------------------------
// DebugAllMsg
//---------------------------------------------------------------
void  DebugAllMsg(const char *msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(const char *msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(const char *msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(const char *msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(string msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(string msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(string msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void  DebugAllMsg(string msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}
#endif

//---------------------------------------------------------------
// ErrMsg
//---------------------------------------------------------------
void ErrMsg(const char *msg, const char *arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(const char *msg, string arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(const char *msg, int arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(const char *msg, double arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(string msg, const char *arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(string msg, string arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(string msg, int arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ErrMsg(string msg, double arg, int status) {
  cerr << msg << " : " << arg <<  " : (" << status << ")\n";
}

//---------------------------------------------------------------
// ConsoleMsg
//---------------------------------------------------------------
void ConsoleMsg(const char *msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(const char *msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(const char *msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(const char *msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(string msg, const char *arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(string msg, string arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(string msg, int arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

void ConsoleMsg(string msg, double arg, int status) {
  cout << msg << " : " << arg <<  " : (" << status << ")\n";
}

//---------------------------------------------------------------
// PrintKeywordMap
//---------------------------------------------------------------
void PrintKeywordMap(map<string, FCL_keyword*> *keywords) {
  // print the map
  // Note that map->first returns the key, map->second returns the value
  // Define a map iterator 
  map<string, FCL_keyword*> :: const_iterator ki;
  for ( ki = keywords->begin(); ki != keywords->end(); ++ki ) {
    cout << ki->first << " : " << ki->second->index << " : " 
	 << ki->second->meaning << "\n";
  }
}

//---------------------------------------------------------------
// PrintInputVariableMap
//---------------------------------------------------------------
void PrintInputVariableMap(map<string, FuzzyInputClass*> *fic) {
  map<string, FuzzyInputClass*> :: const_iterator imi;
  map<string, FuzzyInputTerm*>  :: const_iterator iti;
  map<string, double>           :: const_iterator ivi;
  map<string, FuzzyInputTerm*>  *fitMap;

  cout << "->InputVariables Map\n==========================="
          "=====================\n";
  for ( imi = fic->begin(); imi != fic->end(); ++imi ) {
    cout << "FuzzyInputClass: " << imi->first;
    cout << " varName: " << imi->second->varName;
    cout << " varType: " << imi->second->varType << "\n";
    fitMap = &(imi->second->InputTerms);

    for ( iti = fitMap->begin(); iti != fitMap->end(); ++iti ) {
      cout << "\tFuzzyInputTerm: " << iti->first << "\n";
      cout << "\tvarName: " << iti->second->varName ;
      cout << " termName: " << iti->second->termName;
      cout << " termType: ";

      switch (iti->second->termType) {
      case Trapezoid:
	cout << "Trapezoid\n";
	break;
      case Triangle:
	cout << "Triangle\n";
	break;
      case Ramp:
	cout << "Ramp\n";
	break;
      case Rectangle:
	cout << "Rectangle\n";
	break;
      case Singleton:
	cout << "Singleton\n";
	break;
      default:
	ErrMsg("Failed to find termType for Output term", 
	       iti->second->termName, -1);
      };

      if ( iti->second->termType != Singleton ) {
	cout << "\t";
	vector<XY*>::const_iterator xyi;
	for ( xyi = iti->second->xy.begin(); 
	      xyi != iti->second->xy.end(); ++xyi ) {
	  cout  << " (" << (*xyi)->x << ", " << (*xyi)->y << ") ";
	}
      }
      cout << "\n";
    }
  }
  cout << "============================="
          "===================\n<-InputVariables Map\n";
}

//---------------------------------------------------------------
// PrintOutputVariableMap
//---------------------------------------------------------------
void PrintOutputVariableMap(map<string, FuzzyOutputClass*> *foc) {
  map<string, FuzzyOutputClass*> :: const_iterator omi;
  map<string, FuzzyOutputTerm*>  :: const_iterator oti;
  map<string, FuzzyOutputTerm*>  *fotMap;

  FuzzyOutputTerm*  lpFOT;
  FuzzyOutputClass* lpFOC;

  cout << "->OutputVariables Map\n=================="
          "==============================\n";
  for ( omi = foc->begin(); omi != foc->end(); ++omi ) {
    lpFOC = omi->second;
    cout << "FuzzyOutputClass: " << omi->first;
    cout << " varName: " << lpFOC->varName;
    cout << " varType: " << lpFOC->varType << "\n";
    cout << "\tACCU: "   << lpFOC->accumulation->keyword;
    cout << " METHOD: "  << lpFOC->method->keyword;
    if (lpFOC->defaultNC) {
      cout << " DEFAULT: " << lpFOC->defaultNC->keyword;
    }
    else {
      cout << " DEFAULT: " << lpFOC->defaultOut;
    }
    cout << " RANGE: ("  << lpFOC->minOut << ", " << lpFOC->maxOut << ")\n";

    fotMap = &(lpFOC->OutputTerms);
    for ( oti = fotMap->begin(); oti != fotMap->end(); ++oti ) {
      lpFOT = oti->second;
      cout << "\tFuzzyOutputTerm: " << oti->first << "\n";
      cout << "\tvarName: "         << lpFOT->varName;
      cout << " termName: "         << lpFOT->termName;
      cout << " termType: ";
      switch (lpFOT->termType) {
      case Trapezoid:
	cout << "Trapezoid\n";
	break;
      case Triangle:
	cout << "Triangle\n";
	break;
      case Ramp:
	cout << "Ramp\n";
	break;
      case Rectangle:
	cout << "Rectangle\n";
	break;
      case Singleton:
	cout << "Singleton\n";
	break;
      default:
	ErrMsg("Failed to find termType for Output term", lpFOT->termName, -1);
      };

      if ( lpFOT->termType == Singleton ) {
	cout << "\tsingleton: " << lpFOT->singleton.x << ", " 
	     << lpFOT->singleton.y;
      }
      else {
	cout << "\t";
	vector<XY*>::const_iterator xyi;
	for ( xyi = lpFOT->xy.begin(); xyi != lpFOT->xy.end(); ++xyi ) {
	  cout  << " (" << (*xyi)->x << ", " << (*xyi)->y << ") ";
	}
      }
      cout << "\n";
    }
  }
  cout << "==========================="
          "=====================\n<-OutputVariables Map\n";
}

//---------------------------------------------------------------
// PrintRuleMap
//---------------------------------------------------------------
void PrintRuleMap(map<string, FuzzyRuleClass*> *frc) {
  map<string, FuzzyRuleClass*> :: const_iterator rmi;
  vector<Condition*>  :: iterator condi;
  vector<Conclusion*> :: iterator concli;
  vector<SubCondition*> :: iterator subCondAndi;
  vector<SubCondition*> :: iterator subCondOri;

  cout << "->Rules Map\n================================================\n";
  for ( rmi = frc->begin(); rmi != frc->end(); ++rmi ) {
    cout << "FuzzyRuleClass: " << rmi->first;
    cout << " ruleName: "   << rmi->second->ruleName << "\n";
    cout << "\tAND: "       << rmi->second->andMethod->keyword;
    cout << " OR: "         << rmi->second->orMethod->keyword;
    cout << " ACT METHOD: " << rmi->second->actMethod->keyword << "\n";
    cout << "\tConditions:\n";
    for ( condi =  rmi->second->Conditions.begin(); 
	  condi != rmi->second->Conditions.end(); ++condi ) {
      for ( subCondAndi =  (*condi)->AND_SubConditions.begin(); 
	    subCondAndi != (*condi)->AND_SubConditions.end(); ++subCondAndi) {
	cout << "\tAND SubCondition:";
	cout << " InputVar: "   << (*subCondAndi)->inputVariable->varName;
	cout << " InputTerm: "  << (*subCondAndi)->inputFuzzifyTerm->termName 
	     << "\n";
	if ( (*subCondAndi)->notCondition ) {
	  cout << "\tNOT Condition: " << (*subCondAndi)->notCondition << "\n";
	}
	if ( (*subCondAndi)->notTerm ) {
	  cout << "\tNOT Term: "       << (*subCondAndi)->notTerm << "\n";
	}
      }
      for ( subCondOri =  (*condi)->OR_SubConditions.begin(); 
	    subCondOri != (*condi)->OR_SubConditions.end(); ++subCondOri) {
	cout << "\tOR SubCondition:";
	cout << " InputVar: "   << (*subCondOri)->inputVariable->varName;
	cout << " InputTerm: "  << (*subCondOri)->inputFuzzifyTerm->termName 
	     << "\n";
	if ( (*subCondOri)->notCondition ) {
	  cout << "\tNOT Condition: " << (*subCondOri)->notCondition << "\n";
	}
	if ( (*subCondOri)->notTerm ) {
	  cout << "\tNOT Term: "       << (*subCondOri)->notTerm << "\n";
	}
      }
    }
    cout << "\tConclusions:\n";
    for ( concli = rmi->second->Conclusions.begin();
	  concli != rmi->second->Conclusions.end(); ++concli ) {
      cout << "\tConclusion:";
      cout << " OutputVar: "  << (*concli)->outputVariable->varName;
      cout << " OutputTerm: " << (*concli)->outputDefuzzify->termName;
      cout << " weight: "     << (*concli)->weight;
      cout << "\n";
    }
  }
  cout << "================================================\n<-Rules Map\n";
}

//---------------------------------------------------------------
// PrintFuzzifiedInput
//---------------------------------------------------------------
void PrintFuzzifiedInput(map<string, FuzzyInputClass*> *fic) {

  map<string, FuzzyInputClass*> :: const_iterator imi;
  map<string, FuzzyInputTerm*>  :: const_iterator iti;

  FuzzyInputTerm*  lpFIT;
  FuzzyInputClass* lpFIC;

  cout << "->FuzzifiedInput\n======================="
          "=========================\n";
  for ( imi = fic->begin(); imi != fic->end(); ++imi ) {
    lpFIC = imi->second;
    cout << "Input Variable: " << lpFIC->varName << "\n";

    for ( iti = lpFIC->InputTerms.begin(); 
	  iti != lpFIC->InputTerms.end(); ++iti ) {
      lpFIT = iti->second;
      cout << "\tTerm: " << lpFIT->termName << " is " 
	   << lpFIT->membership <<  "\n";
    }
  }
  cout << "========================"
          "========================\n<-FuzzifiedInput\n";
}

//---------------------------------------------------------------
// PrintDefuzzifiedOutput
//---------------------------------------------------------------
void PrintDefuzzifiedOutput(map<string, FuzzyOutputClass*> *foc) {
  map<string, FuzzyOutputClass*> :: const_iterator omi;

  FuzzyOutputClass* lpFOC;

  cout << "->DefuzzifiedOutput\n======================="
          "=========================\n";
  for ( omi = foc->begin(); omi != foc->end(); ++omi ) {
    lpFOC = omi->second;
    cout << "Output Variable: " << lpFOC->varName;
    cout << " is " << lpFOC->defuzzOut << "\n";
  }
  cout << "======================="
          "=========================\n<-DefuzzifiedOutput\n";
}
