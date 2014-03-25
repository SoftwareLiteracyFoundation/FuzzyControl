#include "FuzzyControl.h"

//--------------------------------------------------------------
// FuzzyOutputClass
//
// Purpose: Constructor for FuzzyOutputClass
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
FuzzyOutputClass::FuzzyOutputClass(FCL_keyword* ACCU_Method,
				   FCL_keyword* METHOD_Method ) : defaultNC(0) {
  
  // set default values
  accumulation = ACCU_Method;
  method       = METHOD_Method;

  defaultOut   = 0.;
  maxOut       =  1.E12; // supposed to be max range of variable type
  minOut       = -1.E12; // supposed to be max range of variable type
  defuzzOut    = 0.;

}
