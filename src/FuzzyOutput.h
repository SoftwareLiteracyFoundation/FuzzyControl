#ifndef Fuzzy_Output_H
#define Fuzzy_Output_H

#include <vector> 
#include <string> 
#include "FCL_Keyword.h"

//---------------------------------------------------------------------
// struct FuzzyOutputTerm
//
// Purpose: Container for an FCL term, the defuzzification function for
//          an output variable
//---------------------------------------------------------------------
struct FuzzyOutputTerm {
  string varName;      // name of the output variable
  string termName;     // name of the term  
  int    termType;     // Trapezoid, Triangle, Ramp, Rectangle, Singleton
  vector<XY*> xy;      // container for term xy points if not Singleton
  XY     singleton;    // output x,y value for Singleton
};

//---------------------------------------------------------------------
// class FuzzyOutput
//
// Purpose: 
//---------------------------------------------------------------------
class FuzzyOutputClass {

 protected:

 public:

  string        varName;      // name of the output variable
  string        varType;      // REAL, INT, ...
  FCL_keyword*  accumulation; // type of ACCU: MAX, BSUM, NSUM
  FCL_keyword*  method;       // defuzzification method: COG, COGS, COA, LM, RM
  FCL_keyword*  defaultNC;    // default output if no rule fired: NC 
  double        defaultOut;   // default output if no rule fired: value
  bool          ruleActive;   // flag for whether or not terms are active
  double        maxOut;       // maximum output, set by RANGE
  double        minOut;       // minimum output, set by RANGE
  double        defuzzOut;    // defuzzified output

  // The output variable term (defuzzification function) map.
  // The key is a string which is the name of the TERM (open, closed...), 
  // the values are a FuzzyOutputTerm struct, one for each term
  map< string, FuzzyOutputTerm* > OutputTerms;

  // Accumulation Map:
  // The key is a string which is the name of the output Term, 
  // the values are a FuzzyOutputTerm, one for each output term.
  // Computed from the accumulation of the activationTerms in the
  // conclusion of each rule.
  map < string, FuzzyOutputTerm* > AccumulationTerms; 

  // FuzzyOutput Methods
  FuzzyOutputClass( FCL_keyword* accuMethod, FCL_keyword* Method );

};

#endif
