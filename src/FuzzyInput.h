#ifndef Fuzzy_Input_H
#define Fuzzy_Input_H

#include <vector> 
#include <string> 

//------------------------------------------------------------
// struct XY
//
// Purpose: Container for x,y points of term definition
//------------------------------------------------------------
struct XY {
  double x;   // x oridinate of point in membership function
  double y;   // y abcissa of point in membership function
};

//------------------------------------------------------------
// struct FuzzyInputTerm
// 
// Purpose: Container for an FCL term, the membership function for an
//          input variable
//------------------------------------------------------------
struct FuzzyInputTerm {
  string varName;    // name of the input variable
  string termName;   // name of the term (membership function) (redundant)
  int    termType;   // Trapezoid, Triangle, Ramp, Rectangle, Singleton
  vector<XY*> xy;    // container for xy 'points' of term
  double membership; // fuzzification value for input 
};

//------------------------------------------------------------
// class FuzzyInputClass
//
// Purpose: 
//------------------------------------------------------------
class FuzzyInputClass {

 protected:

 public:

  string varName;   // name of the input variable
  string varType;   // type: REAL, INT...

  // The input variable term (membership function) map.
  // The key is a string which is the name of the TERM (hot, cold...), 
  // the values are a FuzzyInputTerm struct, one for each term
  map< string, FuzzyInputTerm* > InputTerms; 

  // FuzzyInput Methods
  FuzzyInputClass();

  int FuzzifyTrapezoid( FuzzyInputTerm* fit, double inputValue );
  int FuzzifyTriangle ( FuzzyInputTerm* fit, double inputValue );
  int FuzzifyRamp     ( FuzzyInputTerm* fit, double inputValue );
  int FuzzifyRectangle( FuzzyInputTerm* fit, double inputValue );
  int FuzzifySingleton( FuzzyInputTerm* fit, double inputValue );

};

#endif
