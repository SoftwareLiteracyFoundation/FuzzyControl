#include "FuzzyControl.h"

//--------------------------------------------------------------
// Defuzzification
//
// Purpose: 
//
// Arguments: 
//           
// Return: status  
//--------------------------------------------------------------
int FuzzyControlClass::Defuzzification() {

  int status = 0;

  double defuzz = 0.;

  FuzzyOutputClass* lpFOC;
  FuzzyOutputTerm*  lpFAT;

  map <string, FuzzyOutputClass*>:: iterator foi; // OutputVariables
  map <string, FuzzyOutputTerm*> :: iterator ati; // AccumulationTerms
  
  DebugMsg("Defuzzification", "", status);

  // Each accumulationTerm is a map of <string, FuzzyOutputTerm*> pairs, where
  // the key is the name of the output term, and the value is a pointer to a 
  // FuzzyOutputTerm struct. 
  // So the accumulationTerms are a fuzzy set of the output value.
  // If the output terms are Singletons, then all terms of the set should be 
  // Singletons since the defuzzification methods can't mix areal terms with
  // Singletons.
  // Iterate through output variables
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    bool singletonTerms = false;
    lpFOC = foi->second;
    // Iterate through AccumulationTerms, see if any are singleton
    for ( ati = lpFOC->AccumulationTerms.begin(); 
	  ati != lpFOC->AccumulationTerms.end(); ++ati ) {
      lpFAT = ati->second;
      if  ( lpFAT->termType == Singleton ) {
	singletonTerms = true;
      }
    }
    if ( singletonTerms ) {
      // Iterate through AccumulationTerms, make sure all are singleton
      for ( ati = lpFOC->AccumulationTerms.begin(); 
	    ati != lpFOC->AccumulationTerms.end(); ++ati ) {
	lpFAT = ati->second;
	if  ( lpFAT->termType != Singleton ) {
	  status = -1;
	  ErrMsg("Defuzzification(): Mixture of singleton terms "
		 "with fuzzy terms.",
		 lpFOC->varName + " IS " + lpFAT->termName, status);
	  return status;
	}
      }
    } // if ( singletonTerms )
  } // Iterate through output variables

  // There may be a problem here in how the accumulation terms are considered. 
  // Each accumulation is considered in turn, instead of as an integrated whole
  // fuzzy set. This means that overlap areas between adjacent terms will be
  // accounted for twice in the defuzzification. I don't know if this a concern
  // or not.

  // For each output variable, iterate through the AccumulationTerms
  // and apply the Defuzzification method to set the defuzzOut value

  // Iterate through output variables
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    DebugMsg("\tOutput", lpFOC->varName, status);

    if ( lpFOC->ruleActive ) {

      double uSum   = 0.; // integral of membership values for all terms
      double U_uSum = 0.; // integral of variable Output * membership values
      double sup    = 0.; // supremum (largest value) for all terms
      double inf    = 0.; // infinum (smallest value) for all terms

      // Iterate through AccumulationTerms
      for ( ati = lpFOC->AccumulationTerms.begin(); 
	    ati != lpFOC->AccumulationTerms.end(); ++ati ) {
	lpFAT = ati->second;
	DebugMsg("\tTerm ", lpFAT->termName, status);

	double alpha  = 0.; // slope of interpolated section

	switch ( lpFAT->termType ) {
        case Trapezoid:
	  #ifdef DEBUG // JP
	  cout << "\tTrapezoid: (" << lpFAT->xy[0]->x << ", " 
	       << lpFAT->xy[0]->y << ") ";
	  cout << "(" << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y << ") ";
	  cout << "(" << lpFAT->xy[2]->x << ", " << lpFAT->xy[2]->y << ") ";
	  cout << "(" << lpFAT->xy[3]->x << ", " << lpFAT->xy[3]->y << ") ";
	  cout << "\n";
	  #endif // JP

	  if ( lpFAT->xy.size() == 4 ) {
	    if ( (lpFAT->xy[1]->x <= lpFAT->xy[0]->x) or 
		 (lpFAT->xy[3]->x <= lpFAT->xy[2]->x) ) {
	      status = -1;
	      ErrMsg("Defuzzification: Invalid Trapezoid ordinates", 
		     lpFAT->termName, status);
	      break;
	    }
	    // Area of triangles, in two halves, plus area of rectangle
	    uSum += lpFAT->xy[1]->y/2. * (lpFAT->xy[1]->x - lpFAT->xy[0]->x) + 
	            lpFAT->xy[2]->y/2. * (lpFAT->xy[3]->x - lpFAT->xy[2]->x) +
	            lpFAT->xy[2]->y    * (lpFAT->xy[2]->x - lpFAT->xy[1]->x);
	    // Rectangle contribution
	    U_uSum += lpFAT->xy[1]->y / 2. * 
		      (lpFAT->xy[2]->x * 
		       lpFAT->xy[2]->x - lpFAT->xy[1]->x * lpFAT->xy[1]->x);
	    // Rising triangle contribution
	    alpha = (lpFAT->xy[1]->y - lpFAT->xy[0]->y) / 
	            (lpFAT->xy[1]->x - lpFAT->xy[0]->x);

	    U_uSum += (lpFAT->xy[0]->y - alpha * lpFAT->xy[0]->x) * 
		      (lpFAT->xy[1]->x * lpFAT->xy[1]->x - 
		       lpFAT->xy[0]->x * lpFAT->xy[0]->x)/2. + 
		      alpha * ( (lpFAT->xy[1]->x * lpFAT->xy[1]->x * 
				 lpFAT->xy[1]->x -
			         lpFAT->xy[0]->x * lpFAT->xy[0]->x * 
				 lpFAT->xy[0]->x)/3.);
	    // Down slope triangle contribution
	    alpha = (lpFAT->xy[2]->y - lpFAT->xy[3]->y) / 
	            (lpFAT->xy[3]->x - lpFAT->xy[2]->x);

	    U_uSum += (lpFAT->xy[2]->y + alpha * lpFAT->xy[2]->x) * 
		      (lpFAT->xy[3]->x * lpFAT->xy[3]->x - 
		       lpFAT->xy[2]->x * lpFAT->xy[2]->x)/2. - 
		      alpha * ( (lpFAT->xy[3]->x * lpFAT->xy[3]->x * 
				 lpFAT->xy[3]->x -
			         lpFAT->xy[2]->x * lpFAT->xy[2]->x * 
				 lpFAT->xy[2]->x)/3.);
	  }
	  break;

        case Triangle:
	  #ifdef DEBUG // JP
	  cout << "\tTriangle: (" << lpFAT->xy[0]->x 
	       << ", " << lpFAT->xy[0]->y << ") ";
	  cout << "(" << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y << ") ";
	  cout << "(" << lpFAT->xy[2]->x << ", " << lpFAT->xy[2]->y << ") ";
	  if ( lpFAT->xy.size() == 4 ) 
	    cout << "(" << lpFAT->xy[3]->x << ", " << lpFAT->xy[3]->y << ") ";
	  cout << "\n";
	  #endif // JP

	  if ( lpFAT->xy.size() == 3 or lpFAT->xy.size() == 4 ) {
	    if ( lpFAT->xy[1]->x <= lpFAT->xy[0]->x or 
		 lpFAT->xy[2]->x <= lpFAT->xy[1]->x ) {
	      status = -1;
	      ErrMsg("Defuzzification: Invalid Triangle ordinates", 
		     lpFAT->termName, status);
	      break;
	    }
	    // Area of triangle
	    uSum += lpFAT->xy[1]->y * (lpFAT->xy[2]->x - lpFAT->xy[0]->x)/2.;
	    // Rising triangle contribution
	    alpha = (lpFAT->xy[1]->y - lpFAT->xy[0]->y)/(lpFAT->xy[1]->x - 
							 lpFAT->xy[0]->x);

	    U_uSum += (lpFAT->xy[0]->y - alpha * lpFAT->xy[0]->x) * 
		      (lpFAT->xy[1]->x * lpFAT->xy[1]->x - 
		       lpFAT->xy[0]->x * lpFAT->xy[0]->x)/2. + 
		      alpha * ( (lpFAT->xy[1]->x * lpFAT->xy[1]->x * 
				 lpFAT->xy[1]->x -
			         lpFAT->xy[0]->x * lpFAT->xy[0]->x * 
				 lpFAT->xy[0]->x)/3.);
	    // Down slope triangle contribution
	    alpha = (lpFAT->xy[1]->y - lpFAT->xy[2]->y) / 
	            (lpFAT->xy[2]->x - lpFAT->xy[1]->x);

	    U_uSum += (lpFAT->xy[1]->y + alpha * lpFAT->xy[1]->x) * 
		      (lpFAT->xy[2]->x * lpFAT->xy[2]->x - 
		       lpFAT->xy[1]->x * lpFAT->xy[1]->x)/2. - 
		      alpha * ( (lpFAT->xy[2]->x * lpFAT->xy[2]->x * 
				 lpFAT->xy[2]->x -
			         lpFAT->xy[1]->x * lpFAT->xy[1]->x * 
				 lpFAT->xy[1]->x)/3.);
	  }
	  break;

        case Ramp:
	  if ( lpFAT->xy.size() == 2 ) {
	    if ( lpFAT->xy[1]->y > lpFAT->xy[0]->y ) {
	      // UP Ramp
	      if ( lpFAT->xy[1]->x <= lpFAT->xy[0]->x ) {
		status = -1;
		ErrMsg("Defuzzification: Invalid Ramp ordinates", 
		       lpFAT->termName, status);
		break;
	      }
	      // Area of rectangle and up ramp
	      uSum += lpFAT->xy[1]->y    * (lpFOC->maxOut - lpFAT->xy[1]->x) + 
		      lpFAT->xy[1]->y/2. * (lpFAT->xy[1]->x - lpFAT->xy[0]->x);
	      // Rectangle contribution
	      U_uSum += lpFAT->xy[1]->y / 2. * 
		        (lpFOC->maxOut * lpFOC->maxOut - 
			 lpFAT->xy[1]->x * lpFAT->xy[1]->x);
	      // Up ramp contribution
	      alpha = (lpFAT->xy[1]->y - lpFAT->xy[0]->y) / 
		      (lpFAT->xy[1]->x - lpFAT->xy[0]->x);

	      U_uSum += (lpFAT->xy[0]->y - alpha * lpFAT->xy[0]->x) * 
		        (lpFAT->xy[1]->x * lpFAT->xy[1]->x - 
			 lpFAT->xy[0]->x * lpFAT->xy[0]->x)/2. + 
			alpha * ( (lpFAT->xy[1]->x * lpFAT->xy[1]->x * 
				   lpFAT->xy[1]->x -
			           lpFAT->xy[0]->x * lpFAT->xy[0]->x * 
				   lpFAT->xy[0]->x)/3.);
	    }
	    else {
	      // DOWN Ramp
	      if ( lpFAT->xy[1]->x <= lpFAT->xy[0]->x ) {
		status = -1;
		ErrMsg("Defuzzification: Invalid Ramp ordinates", 
		       lpFAT->termName, status);
		break;
	      }
	      // Area of rectangle and down ramp
	      uSum += lpFAT->xy[0]->y    * (lpFAT->xy[0]->x - lpFOC->minOut) + 
		      lpFAT->xy[0]->y/2. * (lpFAT->xy[1]->x - lpFAT->xy[0]->x);
	      // Rectangle contribution
	      U_uSum += lpFAT->xy[0]->y / 2. * 
		        (lpFAT->xy[1]->x * lpFAT->xy[1]->x - 
			 lpFOC->minOut * lpFOC->minOut );
	      // Down ramp contribution
	      alpha = (lpFAT->xy[0]->y - lpFAT->xy[1]->y) / 
		      (lpFAT->xy[1]->x - lpFAT->xy[0]->x);

	      U_uSum += (lpFAT->xy[0]->y + alpha * lpFAT->xy[0]->x) * 
		        (lpFAT->xy[1]->x * lpFAT->xy[1]->x - 
			 lpFAT->xy[0]->x * lpFAT->xy[0]->x)/2. - 
			alpha * ( (lpFAT->xy[1]->x * lpFAT->xy[1]->x * 
				   lpFAT->xy[1]->x -
			           lpFAT->xy[0]->x * lpFAT->xy[0]->x * 
				   lpFAT->xy[0]->x)/3.);
	    }
	  }
	  break;

        case Rectangle:
	  // COG 
	  if ( lpFAT->xy.size() == 4 ) {
	    uSum   += lpFAT->xy[1]->y * (lpFAT->xy[2]->x - lpFAT->xy[1]->x);
	    U_uSum += lpFAT->xy[1]->y * (lpFAT->xy[2]->x * lpFAT->xy[2]->x - 
					 lpFAT->xy[1]->x * lpFAT->xy[1]->x)/2.;
	  }
	  break;

        case Singleton:
	  // COGS
	  uSum   += lpFAT->singleton.y;
	  U_uSum += lpFAT->singleton.x * lpFAT->singleton.y;
	  break;

        default:
	  status = -1;
	  ErrMsg("Defuzzification() Invalid accumulation term type", 
		 lpFAT->termName, status);
	  return status;
	};

	DebugMsg("\tU_uSum", U_uSum, status);
	DebugMsg("\tuSum",   uSum,   status);

      } // Iterate through AccumulationTerms
    
      // Apply the defuzzification method to this variable
      if ( lpFOC->method == keyword_COG ) {
	if ( fabs(uSum) > 1.E-9 ) defuzz = U_uSum / uSum;
	else defuzz = 0.;
      }
      else if ( lpFOC->method == keyword_COGS ) {
	if ( fabs(uSum) > 1.E-9 ) defuzz = U_uSum / uSum;
	else defuzz = 0.;
      }
      else if ( lpFOC->method == keyword_COA ) {
	status = -1;
	ErrMsg("Defuzzification() COA method not enabled", 
	       lpFOC->method->keyword, status);
	return status;
      }
      else if ( lpFOC->method == keyword_RM ) {
	status = -1;
	ErrMsg("Defuzzification() RM method not enabled", 
	       lpFOC->method->keyword, status);
	return status;
      }
      else if ( lpFOC->method == keyword_LM ) {
	status = -1;
	ErrMsg("Defuzzification() LM method not enabled", 
	       lpFOC->method->keyword, status);
	return status;
      }
      else {
	status = -1;
	ErrMsg("Defuzzification() Invalid method", 
	       lpFOC->method->keyword, status);
	return status;
      }
    } // if ( lpFOC->ruleActive )

    else {
      // No rule fired, all terms have 0 membership
      if ( lpFOC->defaultNC ) {
	// No Change specified, use the previous value
	defuzz = lpFOC->defuzzOut;
	DebugMsg("\t\tNo rule fired: NC", defuzz, status);
      }
      else {
	// Use the value specified in place of NC
	defuzz = lpFOC->defaultOut;
	DebugMsg("\t\tNo rule fired: defaultOut", defuzz, status);
      }
    }

    // Check the RANGE values for the output value
    if      ( defuzz < lpFOC->minOut ) defuzz = lpFOC->minOut;
    else if ( defuzz > lpFOC->maxOut ) defuzz = lpFOC->maxOut;
    
    // Assign the final defuzzified value for this output variable
    lpFOC->defuzzOut = defuzz;

    DebugMsg("\t\tDefuzzOut", lpFOC->defuzzOut, status);

  } // Iterate through output variables

  return status;
}

//--------------------------------------------------------------
// Accumulation
//
// Purpose: Find the activation/accumulation terms, pass to
//          Accumulate().
//
// Arguments: 
//           
// Return: status  
//--------------------------------------------------------------
int FuzzyControlClass::Accumulation() {

  int status = 0;

  FuzzyOutputTerm*  lpFACT;  // access pointer to activation term
  FuzzyOutputTerm*  lpFACCU; // access pointer to accumulation term
  FuzzyOutputClass* lpFOC;   // access pointer to ouput variable
  FuzzyRuleClass*   lpFRC;   // access pointer to Fuzzy Rule
  Conclusion*       lpConc;  // access pointer to conclusion 
  XY*               lpXY;

  map <string, FuzzyRuleClass*>   :: iterator fri;  // Rules map
  vector<Conclusion*>             :: iterator conci;
  map <string, FuzzyOutputClass*> :: iterator foi;  // Output Variables
  map <string, FuzzyOutputTerm*>  :: iterator ati;  // Accumulation terms
  vector<XY*>                     :: iterator xyi;

  // INFERENCE: ACCUMULATION
  // Combination of the weighted results of the rules into an overall result
  // Accumulation records the MAX, ASUM or BSUM accumulation of each output
  // term over all the rules. The resulting FuzzyOutputTerm accumulationTerm
  // is a struct in the FuzzyOutputClass.
  
  // Clear the accumulation terms from previous iterations
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    for ( ati = lpFOC->AccumulationTerms.begin(); 
	  ati != lpFOC->AccumulationTerms.end(); ++ ati ) {
      lpFACCU = ati->second;
      lpFACCU->singleton.y = 0.;
      for ( xyi = lpFACCU->xy.begin(); xyi != lpFACCU->xy.end();++xyi ) {
	delete *xyi;
      }
      lpFACCU->xy.clear();
    }
  }

  // Accumulate the activationTerm from the conclusion of each rule
  // into the accumulationTerm 

  // Iterate through the output variables
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    DebugMsg("Accumulation:", lpFOC->varName, 0);

    // Iterate through the accumulation terms for this variable
    for ( ati = lpFOC->AccumulationTerms.begin(); 
	  ati != lpFOC->AccumulationTerms.end(); ++ ati ) {
      lpFACCU = ati->second;

      // Iterate through rules to accumulate activation from each conclusion
      for ( fri = Rules.begin(); fri != Rules.end(); ++fri ) {
	lpFRC = fri->second;

	// Iterate through the rule conclusions, match with output variable
	for ( conci = lpFRC->Conclusions.begin(); 
	      conci != lpFRC->Conclusions.end(); ++conci ) {
	  lpConc = *conci;
	  if ( lpConc->outputVariable == lpFOC ) {
	    // If the rule fires this activation term, accumulate it
	    lpFACT = &(lpConc->activationTerm);
	    if ( lpFACT->termName == lpFACCU->termName ) {
	      DebugMsg("\tAccumulate RULE", lpFRC->ruleName + " > " + 
		       lpFACCU->varName + " IS " + lpFACCU->termName, 0);
	      status = Accumulate( lpFOC, lpFACT, lpFACCU );
	      if ( status != 0 ) {
		ErrMsg("Failed to accumulate term", lpFACT->termName, status);
		return status;
	      }
	    }
	  } // Iterate through conclusions
	} // Iterate through rules 
      } // Iterate through the accumulation terms
    } // Iterate through the ouput variables

  } // Iterate through the rule base

  // Check to see if the accumulation resulted in a rule being active, 
  // i.e. there is a non-zero membership term value, set the ruleActive flag 
  // Defuzzification only operates on rules with ruleActive true
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    lpFOC->ruleActive = false;
    for ( ati = lpFOC->AccumulationTerms.begin(); 
	  ati != lpFOC->AccumulationTerms.end(); ++ ati ) {
      lpFACCU = ati->second;
      if ( lpFACCU->termType == Singleton ) {
	if ( fabs(lpFACCU->singleton.y) >= ZERO_TOLERANCE ) {
	  lpFOC->ruleActive = true;
	  continue;
	}
      }
      else {
	for ( xyi = lpFACCU->xy.begin(); xyi != lpFACCU->xy.end(); ++xyi ) {
	  lpXY = *xyi;
	  if ( fabs(lpXY->y) >= ZERO_TOLERANCE ) {
	    lpFOC->ruleActive = true;
	    continue;
	  }
	}
      }
    }
  }
  // It is possible that accumulation combined Trapezoids with Triangles
  // to produce a triangle output, but in a trapezoid containter, the fourth
  // point will have y=0
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    if ( lpFOC->ruleActive ) {
      for ( ati = lpFOC->AccumulationTerms.begin(); 
	    ati != lpFOC->AccumulationTerms.end(); ++ ati ) {
	lpFACCU = ati->second;
	if ( lpFACCU->termType == Trapezoid ) {
	  if ( lpFACCU->xy.size() < 1 ) {
	    ErrMsg( "Accumulation: Trapezoid term conversion failed "
		    " on variable", lpFACCU->varName, -1 );
	    ErrMsg( "Accumulation: Trapezoid term conversion failed "
		    " on term", lpFACCU->termName, -1 );
	    status = -1;
	    break;
	  }
	  if ( fabs(lpFACCU->xy.at(1)->y) > ZERO_TOLERANCE ) {
	    if ( lpFACCU->xy.size() < 3 ) {
	      ErrMsg( "Accumulation: Trapezoid term conversion failed "
		      "on variable", lpFACCU->varName, -1 );
	      ErrMsg( "Accumulation: Trapezoid term conversion failed "
		      "on term", lpFACCU->termName, -1 );
	      status = -1;
	      break;
	    }
	    if ( lpFACCU->xy.at(2)->x == lpFACCU->xy.at(3)->x ) {
	      delete lpFACCU->xy[3];
	      lpFACCU->xy.pop_back();
	      lpFACCU->termType = Triangle;
	    }
	  }
	}
      }
    }
  }
  // Console debug information
  // Print the non-zero accumulation terms if DEBUG
  #ifdef DEBUG
  for ( foi = OutputVariables.begin(); foi != OutputVariables.end(); ++foi ) {
    lpFOC = foi->second;
    for ( ati = lpFOC->AccumulationTerms.begin(); 
	  ati != lpFOC->AccumulationTerms.end(); ++ ati ) {
      lpFACCU = ati->second;
      if ( lpFOC->ruleActive ) {
	if ( lpFACCU->termType == Singleton ) {
	  DebugMsg("Final Accumulation:", lpFACCU->varName + " IS " + 
		   lpFACCU->termName, 0);
	  cout << "\t\t(" << lpFACCU->singleton.x 
	       << ", " << lpFACCU->singleton.y << ")\n";
	}
	else {
	  for ( xyi = lpFACCU->xy.begin(); xyi != lpFACCU->xy.end(); ++xyi ) {
	    lpXY = *xyi;
	    if ( xyi == lpFACCU->xy.begin() ) {
	      DebugMsg("Final Accumulation:", lpFACCU->varName + " IS " + 
		       lpFACCU->termName, 0);
	      cout << "\t\t";
	    }
	    cout << "(" << lpXY->x << ", " << lpXY->y << ")  ";
	  }
	  if (lpFACCU->xy.size()) cout << "\n";
	}
      }
    }
  }
  #endif

  return status;
}

//--------------------------------------------------------------
// Accumulate
//
// Purpose: Accumulate the rule conclusion activationTerms into an
//          accumulationTerm.
//
// Arguments: 
//           
// Return: status  
//--------------------------------------------------------------
int FuzzyControlClass::Accumulate( FuzzyOutputClass* lpFOC, 
				   FuzzyOutputTerm*  lpFACT,
				   FuzzyOutputTerm*  lpFACCU ) {

  int  status = 0;
  int  i      = 0;

  XY*              lpXY;
  FuzzyOutputTerm* lpFOT;  // access pointer to FCL defined term

  vector<XY*> :: iterator xyi;
      
  if ( lpFOC->accumulation == keyword_Max  or 
       lpFOC->accumulation == keyword_Bsum or
       lpFOC->accumulation == keyword_Nsum ) {
    DebugAllMsg("\t\tAccumulate with ACCU=", 
		lpFOC->accumulation->keyword, status );
  }
  else {
    status = -1;
    ErrMsg("Accumulate(): invalid Accumulation", 
	   lpFOC->accumulation->keyword, status);
    return status;
  }

  // If it doesn't exist, create the XY struct terms for the accumulation
  // Just copy the activation term, since it's the 1st and maybe only one
  if ( lpFACCU->termType != Singleton and not lpFACCU->xy.size() ) {
    for ( xyi = lpFACT->xy.begin(); xyi != lpFACT->xy.end(); ++xyi ) {
      XY* lpXY = new XY;
      if ( not lpXY ) {
	status = -1; ErrMsg("Accumulate() Failed to alloc XY", "", status);
	return status;
      }
      lpXY->x = lpFACT->xy[i]->x;
      lpXY->y = lpFACT->xy[i]->y;
      lpFACCU->xy.push_back(lpXY);
      i++;
    }
    return status;
  }

  // Find the matching FCL defined output term
  if ( lpFOC->OutputTerms.find(lpFACCU->termName) == 
       lpFOC->OutputTerms.end() ) {
    status = -1;
    ErrMsg("Accumulate(): Failed to find FCL output term", 
	   lpFACCU->termName, status);
    return status;
  }
  lpFOT = lpFOC->OutputTerms[lpFACCU->termName];

  // The activationTerm will control the type of the accumulation term, since
  // activation may have converted an ouput triangle term into a trapezoid.
  switch ( lpFACT->termType ) {

    case Trapezoid:
      // set the term type
      lpFACCU->termType = Trapezoid;

      if ( lpFACCU->xy.size() == 3 ) {
	// It was a Triangle Output term that Activation changed to a Trapezoid
	XY* lpXY = new XY;
	if ( not lpXY ) {
	  status = -1; ErrMsg("Accumulate() Failed to alloc XY for Trapezoid", 
			      "", status);
	  return status;
	}
	lpXY->x = lpFACT->xy[3]->x;
	lpXY->y = lpFACT->xy[3]->y;
	lpFACCU->xy.push_back(lpXY);
      }
      if ( lpFACCU->xy.size() != 4 ) {
	status = -1;
	ErrMsg("Accumulate() Invalid number of points for Trapezoid", 
	       lpFACT->termName, status);
	return status;
      }

      // There is an existing accumulation term
      // The first and fourth points are the same ordinates, but
      // the abcissa may have changed
      lpFACCU->xy[0]->x = lpFACT->xy[0]->x;
      lpFACCU->xy[3]->x = lpFACT->xy[3]->x;
      // Handle the abcissa's
      if ( lpFOC->accumulation == keyword_Max ) { // MAX
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = max(lpFACCU->xy[i]->y, lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Bsum ) { // BSUM
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = min(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Nsum ) { // NSUM
	// JP This NSUM isn't strictly correct, the denominator should form
	// a global max in the second argument to max(). See FCL pg 14.
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = (lpFACCU->xy[i]->y + lpFACT->xy[i]->y) / 
	                      (max(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y));
	}
      }
      // JP These x ordinates are strictly correct only for MAX
      if ( lpFACCU->xy[1]->y > lpFACT->xy[1]->y ) {
	lpFACCU->xy[1]->x = lpFACCU->xy[1]->x;
	lpFACCU->xy[2]->x = lpFACCU->xy[2]->x;
      }
      else {
	lpFACCU->xy[1]->x = lpFACT->xy[1]->x;
	lpFACCU->xy[2]->x = lpFACT->xy[2]->x;
      }
      break;

    case Triangle:
      // set the term type
      lpFACCU->termType = Triangle;

      // if lpFACCU->xy.size() == 4
      // There was a triangle Output term that Activation changed to Trapezoid,
      // but a subsequent Triangle term that was not converted to a trapezoid is
      // encountered (it is type Triangle, but has 4 xy points), can just ignore
      // the extra point since it doesn't have any valid info for this term
      if ( lpFACCU->xy.size() < 3 ) {
	status = -1;
	ErrMsg("Accumulate() Invalid number of points for Triangle", 
	       lpFACT->termName, status);
	return status;
      }
	
      // There is an existing accumulation term
      // Use the same ordinates, this may not be strictly right
      lpFACCU->xy[0]->x = lpFACT->xy[0]->x;
      lpFACCU->xy[1]->x = lpFACT->xy[1]->x;
      lpFACCU->xy[2]->x = lpFACT->xy[2]->x;
      // Handle the abcissa's
      if ( lpFOC->accumulation == keyword_Max ) { // MAX
	for ( i = 0; i<=2; i++ ) {
	  lpFACCU->xy[i]->y = max(lpFACCU->xy[i]->y, lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Bsum ) { // BSUM
	for ( i = 0; i<=2; i++ ) {
	  lpFACCU->xy[i]->y = min(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Nsum ) { // NSUM
	for ( i = 0; i<=2; i++ ) {
	  lpFACCU->xy[i]->y = (lpFACCU->xy[i]->y + lpFACT->xy[i]->y) / 
	                      (max(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y));
	}
      }
      break;

    case Ramp:
      // set the term type
      lpFACCU->termType = Ramp;

      if ( lpFACCU->xy.size() != 2 ) {
	status = -1;
	ErrMsg("Accumulate() Invalid number of points for Ramp", 
	       lpFACT->termName, status);
	return status;
      }

      // There is an existing accumulation term
      // abcissa values
      if ( lpFOC->accumulation == keyword_Max ) { // MAX
	for ( i = 0; i<=1; i++ ) {
	  lpFACCU->xy[i]->y = max(lpFACCU->xy[i]->y, lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Bsum ) { // BSUM
	for ( i = 0; i<=1; i++ ) {
	  lpFACCU->xy[i]->y = min(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Nsum ) { // NSUM
	for ( i = 0; i<=1; i++ ) {
	  lpFACCU->xy[i]->y = (lpFACCU->xy[i]->y + lpFACT->xy[i]->y) / 
	                      (max(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y));
	}
      }
      if ( lpFACT->xy[1]->y > lpFACT->xy[0]->y ) {
	// Ramp is UP
	// the first point has same ordinate
	lpFACCU->xy[0]->x = lpFACT->xy[0]->x;
	// second point depends on which is higher
	if ( lpFACCU->xy[1]->y > lpFACT->xy[1]->y ) {
	  // point doesn't change
	}
	else {
	  lpFACCU->xy[1]->x = lpFACT->xy[1]->x;
	}
      }
      else {
	// Ramp is DOWN
	// the second point has same ordinate
	lpFACCU->xy[1]->x = lpFACT->xy[1]->x;
	// first point depends on which is higher
	if ( lpFACCU->xy[0]->y > lpFACT->xy[0]->y ) {
	  // point doesn't change
	}
	else {
	  lpFACCU->xy[0]->x = lpFACT->xy[0]->x;
	}
      }
      break;

    case Rectangle:
      // set the term type
      lpFACCU->termType = Rectangle;

      if ( lpFACCU->xy.size() != 4 ) {
	status = -1;
	ErrMsg("Accumulate() Invalid number of points for Rectangle", 
	       lpFACT->termName, status);
	return status;
      }

      // There is an existing accumulation term
      // The ordinates don't change
      for ( i = 0; i<=3; i++ ) {
	lpFACCU->xy[i]->x = lpFACT->xy[i]->x;
      }
      // Handle the abcissa's
      if ( lpFOC->accumulation == keyword_Max ) { // MAX
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = max(lpFACCU->xy[i]->y, lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Bsum ) { // BSUM
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = min(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y);
	}
      }
      else if ( lpFOC->accumulation == keyword_Nsum ) { // NSUM
	for ( i = 0; i<=3; i++ ) {
	  lpFACCU->xy[i]->y = (lpFACCU->xy[i]->y + lpFACT->xy[i]->y) / 
	                      (max(1., lpFACCU->xy[i]->y + lpFACT->xy[i]->y));
	}
      }
      break;

    case Singleton:
      // set the term type
      lpFACCU->termType = Singleton;
      // Make sure there aren't any xy points
      for ( xyi = lpFACT->xy.begin(); xyi != lpFACT->xy.end(); ++xyi ) {
	delete *xyi;
      }
      lpFACT->xy.clear();

      lpFACCU->singleton.x = lpFACT->singleton.x;

      if ( lpFOC->accumulation == keyword_Max ) { // MAX
	lpFACCU->singleton.y = max( lpFACCU->singleton.y, lpFACT->singleton.y );
      }
      else if ( lpFOC->accumulation == keyword_Bsum ) { // BSUM
	lpFACCU->singleton.y = min( 1., lpFACCU->singleton.y + 
				        lpFACT->singleton.y );
      }
      else if ( lpFOC->accumulation == keyword_Nsum ) { // NSUM
	lpFACCU->singleton.y = ( lpFACCU->singleton.y + lpFACT->singleton.y ) / 
	                         max( 1., lpFACCU->singleton.y + 
				          lpFACT->singleton.y );
      }
      break;
    
    default:
      status = -1;
      ErrMsg("Accumulate() Invalid term type", lpFACT->termName, status);
      return status;
    
  };

  return status;
}

//--------------------------------------------------------------
// Activation
//
// Purpose: 
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::Activation() {

  int status = 0;
  FuzzyRuleClass* lpFRC; // access pointer to Fuzzy Rule

  map <string, FuzzyRuleClass*> :: iterator fri; // Rules map

  // INFERENCE: ACTIVATION
  // Activation (assign the value) of the IF-THEN conclusion
  // For each rule, the result of the condition aggregation is convolved through
  // the output term(s) with the ACT operator (PROD or MIN).
  // This produces a modified FuzzyOutputTerm. The resulting FuzzyOutputTerm
  // is the activationTerm struct in the rule Conclusion struct.
  // Iterate through the rule base
  for ( fri = Rules.begin(); fri != Rules.end(); ++fri ) {
    lpFRC = fri->second;
    DebugMsg("Activation: Rule:", lpFRC->ruleName, 0);
    
    status = lpFRC->Activate(); // Activate() in FuzzyRules.cc
    if ( status != 0 ) {
      ErrMsg("Failed to Activate rule", lpFRC->ruleName, status);
      return status;
    }
  } // Iterate through the rule base

  return status;
}

//--------------------------------------------------------------
// Aggregation
//
// Purpose: 
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::Aggregation() {

  int status = 0;
  FuzzyRuleClass* lpFRC; // access pointer to Fuzzy Rule

  map <string, FuzzyRuleClass*> :: iterator fri; // Rules map

  // INFERENCE: AGGREGATION
  // Determine degree of conformance of the rule conditions from the degree of
  // membership of the condition terms, i.e. consolodate the conditions into
  // a single membership value based on AND/OR of sub-conditions.
  // Method: for each rule, find the AND (Min) or OR (Max) combination of rule
  // conditions to result in a single condition (premise) membership number.
  // The AND/OR operators are defined in the RULEBLOCK, and have a keyword 
  // identifier in the FuzzyRuleClass objects andMethod, orMethod
  // Iterate through the rule base
  for ( fri = Rules.begin(); fri != Rules.end(); ++fri ) {
    lpFRC = fri->second;
    DebugAllMsg("Aggregation: Rule:", lpFRC->ruleName, 0);

    // Find the ANDSubCondition subResult by combining the terms with andMethod
    status = lpFRC->AND_SubConditions();
    if ( status != 0 ) {
      ErrMsg("Failed to Aggregate AND SubConditions in rule", 
	     lpFRC->ruleName, status);
      return status;
    }

    // Find the ORSubCondition subResult by combining the terms with orMethod
    status = lpFRC->OR_SubConditions();
    if ( status != 0 ) {
      ErrMsg("Failed to Aggregate AND SubConditions in rule", 
	     lpFRC->ruleName, status);
      return status;
    }
  } // Iterate through the rule base

  return status;
}

//--------------------------------------------------------------
// Fuzzification
//
// Purpose: 
//
// Arguments: index in InputData vector
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::Fuzzification(int inputDataIndex) {

  int status = 0;
  string varName;
  double dataValue;
  map<string, vector<double>* > :: const_iterator idi; // InputData

  // FUZZIFICATION
  // Conversion of input values to linguistic variables
  // Iterate through the InputData variables
  for ( idi = InputData.begin(); idi != InputData.end(); ++idi ) {
    // process the variable with the ith data point from it's vector
    if ( (*idi->second).size() < inputDataIndex ) {
      status = -1;
      ErrMsg("Fuzzification()", "inputDataIndex out of range", status);
      return status;
    }
    varName   = idi->first;
    dataValue = (*idi->second)[inputDataIndex];
      
    // Assign the fuzzified input data value to the 
    // FuzzyInputClass.InputTerms.membership
    status = FuzzifyInput( varName, dataValue );
    if ( status != 0 ) {
      ErrMsg( "Failed to fuzzify input variable", varName, status );
      ErrMsg( "Failed to fuzzify input value", dataValue, status );
      return status;
    }
  }
  return status;
}

//--------------------------------------------------------------
// FuzzifyInput
//
// Purpose: Assign the membership function values to each term
//          of a fuzzy input variable (do the fuzzification)
//
// Arguments: varName, inputValue 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::FuzzifyInput( string varName, double inputValue ) {

  int status = 0;

  // Find the input variable in the InputVariables Map
  if ( InputVariables.find(varName) != InputVariables.end() ) {
    DebugMsg("Fuzzify variable", varName, 0);
    DebugAllMsg("\tVariable value", inputValue, 0);
  }
  else {
    status = -1;
    ErrMsg("Failed to find Input Variable from Data Input in database", 
	   varName, status);
    return status;
  }
  FuzzyInputClass* lpFIC = InputVariables[varName];

  // For each term in the InputTerms map, assign the membership value
  map<string, FuzzyInputTerm*> :: const_iterator fti;

  for ( fti = lpFIC->InputTerms.begin(); 
	fti != lpFIC->InputTerms.end(); ++fti ) {
    DebugMsg("\tFuzzify Input Term", fti->first, 0);

    switch (fti->second->termType) {
    case Trapezoid:
      status = lpFIC->FuzzifyTrapezoid(fti->second, inputValue);
      break;
    case Triangle:
      status = lpFIC->FuzzifyTriangle(fti->second, inputValue);      
      break;
    case Ramp:
      status = lpFIC->FuzzifyRamp(fti->second, inputValue);
      break;
    case Rectangle:
      status = lpFIC->FuzzifyRectangle(fti->second, inputValue);
      break;
    case Singleton:
      status = lpFIC->FuzzifySingleton(fti->second, inputValue);
      break;
    default:
      status = -1;
      ErrMsg("Failed to find termType for input term", fti->first, status);
      return status;
    };
  }

  return status;
}

//--------------------------------------------------------------
// FuzzyControlClass
//
// Purpose: Constructor for FuzzyControlClass.
//          Set the FCLFileName, create the FCL keywords map.
//          Initialize the keyword pointers.
//
// Arguments: 
//           FCLFileName
//           
// Return:   
//--------------------------------------------------------------
FuzzyControlClass::FuzzyControlClass( string fileName, 
				      string inputDelimeters,
				      string inputDataLabel ) {

  // Initialize version 
  FCLL_Version* FCLLVersion = new FCLL_Version();
  FCLLVersion->SetVersionNumber("1", "0", "1");
  FCLLVersion->SetBuildDate("None");
  FCLLVersion->SetReleaseDate("None");
  FCLLVersion->SetVersionString();

  // Initialize FCL file name
  FCLFileName = fileName;

  // Initialize other members
  status = 0;

  if ( not inputDelimeters.length() ) {
    // Assume the input data file is .csv format
    InputDelimeters() = ","; // " ,\t;:";
  }
  else {
    InputDelimeters() = inputDelimeters;
  }

  if ( inputDataLabel.length() ) {
    InputDataLabel() = inputDataLabel;
  }

  // Initialize the map of FCL keywords
  status = LoadFCLKeywords(keywords);
  if ( status != 0 ) {
    ErrMsg("LoadFCLKeywords() Failed" , "", status);
    return;
  }

  // Echo map to console if DEBUG_ALL
  #ifdef DEBUG_ALL
  PrintKeywordMap(&keywords);
  #endif

  // Initialize pointers for FCL_keyword structs
  status = -1;
  keyword_Accu = FindFCLKeywordFromMap ("ACCU", true);
  if ( !keyword_Accu ) {
    ErrMsg("Failed to find ACCU keyword" , "ACCU", status);
    return;
  }
  keyword_ACT = FindFCLKeywordFromMap ("ACT", true);
  if ( !keyword_ACT ) {
    ErrMsg("Failed to find  keyword" , "ACT", status);
    return;
  }
  keyword_And = FindFCLKeywordFromMap ("AND", true);
  if ( !keyword_And ) {
    ErrMsg("Failed to find  keyword" , "AND", status);
    return;
  }
  keyword_Asum = FindFCLKeywordFromMap ("ASUM", true);
  if ( !keyword_Asum ) {
    ErrMsg("Failed to find  keyword" , "ASUM", status);
    return;
  }
  keyword_Bdif = FindFCLKeywordFromMap ("BDIF", true);
  if ( !keyword_Bdif ) {
    ErrMsg("Failed to find  keyword" , "BDIF", status);
    return;
  }
  keyword_Bsum = FindFCLKeywordFromMap ("BSUM", true);
  if ( !keyword_Bsum ) {
    ErrMsg("Failed to find  keyword" , "BSUM", status);
    return;
  }
  keyword_COA = FindFCLKeywordFromMap ("COA", true);
  if ( !keyword_COA ) {
    ErrMsg("Failed to find  keyword" , "COA", status);
    return;
  }
  keyword_COG = FindFCLKeywordFromMap ("COG", true);
  if ( !keyword_COG ) {
    ErrMsg("Failed to find  keyword" , "COG", status);
    return;
  }
  keyword_COGS = FindFCLKeywordFromMap ("COGS", true);
  if ( !keyword_COGS ) {
    ErrMsg("Failed to find  keyword" , "COGS", status);
    return;
  }
  keyword_Default = FindFCLKeywordFromMap ("DEFAULT", true);
  if ( !keyword_Default ) {
    ErrMsg("Failed to find  keyword" , "DEFAULT", status);
    return;
  }
  keyword_Defuzzify = FindFCLKeywordFromMap ("DEFUZZIFY", true);
  if ( !keyword_Defuzzify ) {
    ErrMsg("Failed to find  keyword" , "DEFUZZIFY", status);
    return;
  }
  keyword_EndDefuzzify = FindFCLKeywordFromMap ("END_DEFUZZIFY", true);
  if ( !keyword_EndDefuzzify ) {
    ErrMsg("Failed to find  keyword" , "END_DEFUZZIFY", status);
    return;
  }
  keyword_EndFuncBlock = FindFCLKeywordFromMap ("END_FUNCTION_BLOCK", true);
  if ( !keyword_EndFuncBlock ) {
    ErrMsg("Failed to find  keyword" , "END_FUNCTION_BLOCK", status);
    return;
  }
  keyword_EndFuzzify = FindFCLKeywordFromMap ("END_FUZZIFY", true);
  if ( !keyword_EndFuzzify ) {
    ErrMsg("Failed to find  keyword" , "END_FUZZIFY", status);
    return;
  }
  keyword_EndOpt = FindFCLKeywordFromMap ("END_OPTIONS", true);
  if ( !keyword_EndOpt ) {
    ErrMsg("Failed to find  keyword" , "END_OPTIONS", status);
    return;
  }
  keyword_EndRule = FindFCLKeywordFromMap ("END_RULEBLOCK", true);
  if ( !keyword_EndRule ) {
    ErrMsg("Failed to find  keyword" , "END_RULEBLOCK", status);
    return;
  }
  keyword_EndVar = FindFCLKeywordFromMap ("END_VAR", true);
  if ( !keyword_EndVar ) {
    ErrMsg("Failed to find  keyword" , "END_VAR", status);
    return;
  }
  keyword_FuncBlock = FindFCLKeywordFromMap ("FUNCTION_BLOCK", true);
  if ( !keyword_FuncBlock ) {
    ErrMsg("Failed to find  keyword" , "FUNCTION_BLOCK", status);
    return;
  }
  keyword_Fuzzify = FindFCLKeywordFromMap ("FUZZIFY", true);
  if ( !keyword_Fuzzify ) {
    ErrMsg("Failed to find  keyword" , "FUZZIFY", status);
    return;
  }
  keyword_IF = FindFCLKeywordFromMap ("IF", true);
  if ( !keyword_IF ) {
    ErrMsg("Failed to find  keyword" , "IF", status);
    return;
  }
  keyword_IS = FindFCLKeywordFromMap ("IS", true);
  if ( !keyword_IS ) {
    ErrMsg("Failed to find  keyword" , "IS", status);
    return;
  }
  keyword_LM = FindFCLKeywordFromMap ("LM", true);
  if ( !keyword_LM ) {
    ErrMsg("Failed to find  keyword" , "LM", status);
    return;
  }
  keyword_Max = FindFCLKeywordFromMap ("MAX", true);
  if ( !keyword_Max ) {
    ErrMsg("Failed to find  keyword" , "MAX", status);
    return;
  }
  keyword_Method = FindFCLKeywordFromMap ("METHOD", true);
  if ( !keyword_Method ) {
    ErrMsg("Failed to find  keyword" , "METHOD", status);
    return;
  }
  keyword_Min = FindFCLKeywordFromMap ("MIN", true);
  if ( !keyword_Min ) {
    ErrMsg("Failed to find  keyword" , "MIN", status);
    return;
  }
  keyword_NC = FindFCLKeywordFromMap ("NC", true);
  if ( !keyword_NC ) {
    ErrMsg("Failed to find  keyword" , "NC", status);
    return;
  }
  keyword_Not = FindFCLKeywordFromMap ("NOT", true);
  if ( !keyword_Not ) {
    ErrMsg("Failed to find  keyword" , "NOT", status);
    return;
  }
  keyword_Nsum = FindFCLKeywordFromMap ("NSUM", true);
  if ( !keyword_Nsum ) {
    ErrMsg("Failed to find  keyword" , "NSUM", status);
    return;
  }
  keyword_Options = FindFCLKeywordFromMap ("OPTIONS", true);
  if ( !keyword_Options ) {
    ErrMsg("Failed to find  keyword" , "OPTIONS", status);
    return;
  }
  keyword_Or = FindFCLKeywordFromMap ("OR", true);
  if ( !keyword_Or ) {
    ErrMsg("Failed to find  keyword" , "OR", status);
    return;
  }
  keyword_Prod = FindFCLKeywordFromMap ("PROD", true);
  if ( !keyword_Prod ) {
    ErrMsg("Failed to find  keyword" , "PROD", status);
    return;
  }
  keyword_Range = FindFCLKeywordFromMap ("RANGE", true);
  if ( !keyword_Range ) {
    ErrMsg("Failed to find  keyword" , "RANGE", status);
    return;
  }
  keyword_RM = FindFCLKeywordFromMap ("RM", true);
  if ( !keyword_RM ) {
    ErrMsg("Failed to find  keyword" , "RM", status);
    return;
  }
  keyword_Rule = FindFCLKeywordFromMap ("RULE", true);
  if ( !keyword_Rule ) {
    ErrMsg("Failed to find  keyword" , "RULE", status);
    return;
  }
  keyword_RuleBlock = FindFCLKeywordFromMap ("RULEBLOCK", true);
  if ( !keyword_RuleBlock ) {
    ErrMsg("Failed to find  keyword" , "RULEBLOCK", status);
    return;
  }
  keyword_Term = FindFCLKeywordFromMap ("TERM", true);
  if ( !keyword_Term ) {
    ErrMsg("Failed to find  keyword" , "TERM", status);
    return;
  }
  keyword_Then = FindFCLKeywordFromMap ("THEN", true);
  if ( !keyword_Then ) {
    ErrMsg("Failed to find  keyword" , "THEN", status);
    return;
  }
  keyword_Var = FindFCLKeywordFromMap ("VAR", true);
  if ( !keyword_Var ) {
    ErrMsg("Failed to find  keyword" , "VAR", status);
    return;
  }
  keyword_VarIn = FindFCLKeywordFromMap ("VAR_INPUT", true);
  if ( !keyword_VarIn ) {
    ErrMsg("Failed to find  keyword" , "VAR_INPUT", status);
    return;
  }
  keyword_VarOut = FindFCLKeywordFromMap ("VAR_OUTPUT", true);
  if ( !keyword_VarOut ) {
    ErrMsg("Failed to find  keyword" , "VAR_OUTPUT", status);
    return;
  }
  keyword_With = FindFCLKeywordFromMap ("WITH", true);
  if ( !keyword_With ) {
    ErrMsg("Failed to find WITH keyword" , "", status);
    return;
  }

  status = 0;
}
