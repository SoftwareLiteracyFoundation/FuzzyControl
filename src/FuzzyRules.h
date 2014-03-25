#ifndef Fuzzy_Rules_H
#define Fuzzy_Rules_H

#include <vector> 
#include <string> 
#include "FCL_Keyword.h"

//---------------------------------------------------------------------
// struct Subcondition
//
// Purpose: Basic container for rule subcondition
//---------------------------------------------------------------------
struct SubCondition {

  FuzzyInputClass* inputVariable;
  FuzzyInputTerm*  inputFuzzifyTerm;

  bool   notCondition; // true if NOT is applied to SubCondition
  bool   notTerm;      // true if NOT applied to inputFuzzifyTerm
  double subResult;    // degree of membership of subcondition
};

//---------------------------------------------------------------------
// struct Condition
//
// Purpose: Basic container for rule condition
//---------------------------------------------------------------------
struct Condition {

  vector<SubCondition*> AND_SubConditions;
  vector<SubCondition*> OR_SubConditions;

  double result; // aggregate degree of membership of condition (premise)
};

//---------------------------------------------------------------------
// struct Conclusion
//
// Purpose: Basic container for rule conclusions
//---------------------------------------------------------------------
struct Conclusion {

  FuzzyOutputClass* outputVariable;
  FuzzyOutputTerm*  outputDefuzzify;

  FuzzyOutputTerm activationTerm;   // ACT term for this rule conclusion

  double weight; // output rule scale factor: WITH (Initialize to 1!)

};

//---------------------------------------------------------------------
// class FuzzyRule
//---------------------------------------------------------------------
class FuzzyRuleClass {

 protected:

 public:

  string ruleName;

  int nSubConclusions; // number of sub-conclusions, after THEN
  int nSubConditions;  // number of sub-conditions, between IF..THEN

  vector<Condition*>  Conditions;  // stack of rule conditions
  vector<Conclusion*> Conclusions; // stack of rule conclusions

  double conditionResult;  // aggregation of conditions

  // methods are represented by their FCL_keyword* 
  FCL_keyword* andMethod; // AND method: MIN, PROD, BDIF
  FCL_keyword* orMethod;  // OR method:  MAX, ASUM, BSUM
  FCL_keyword* actMethod; // ACT method: PROD, MIN

  // local pointers to the keyword pointers in FuzzyControlClass 
  // needed in AND/OR_SubConditions
  FCL_keyword* kwd_Min;
  FCL_keyword* kwd_Max;
  FCL_keyword* kwd_Prod;
  FCL_keyword* kwd_Bdif;
  FCL_keyword* kwd_Asum;
  FCL_keyword* kwd_Bsum;

  // FuzzyRule Methods
  FuzzyRuleClass( FCL_keyword* kwdMin,  FCL_keyword* kwdMax,  
		  FCL_keyword* kwdProd, FCL_keyword* kwdBdiff, 
		  FCL_keyword* kwdAsum, FCL_keyword* kwdBsum );

  int AND_SubConditions();
  int OR_SubConditions();
  int Activate();

};

#endif
