#include "FuzzyControl.h"

//--------------------------------------------------------------
// Activate
//
// Purpose: Perform the appropriate ACT operation
//          Valid operations are: MIN, PROD
//
// Arguments: 
//           
// Return: status  
//--------------------------------------------------------------
int FuzzyRuleClass::Activate() {

  int status = 0;

  vector<Conclusion*> :: iterator conci; // Conclusions iterator
  vector<XY*>         :: iterator xyi;

  int i = 0;
  XY*               lpActXY   = 0; // temp pntrs for new ACT term
  XY*               lpActXY_0 = 0; 
  XY*               lpActXY_1 = 0;
  XY*               lpActXY_2 = 0;
  XY*               lpActXY_3 = 0;
  XY*               lpXY      = 0; // p to copy from FCL defined FuzzyOutputTerm
  Conclusion*       lpConc; // pntr to Fuzzy Rule Conclusion struct
  FuzzyOutputClass* lpFOC;  // Fuzzy Output Class
  FuzzyOutputTerm*  lpFOT;  // Fuzzy Output Term
  FuzzyOutputTerm*  lpFAT;  // Fuzzy Activation Term

  bool rampUp   = false;
  bool rampDown = false;
  bool aggregationAbove = false;
  bool aggregationBelow = false;
  
  if ( actMethod != kwd_Min and actMethod != kwd_Prod ) {
    status = -1;
    ErrMsg("Activate() Invalid ACT method", actMethod->keyword, status);
  }

  // For this rule, the result of the condition aggregation is convolved
  // through the output term(s) with the ACT operator (PROD or MIN).
  // This may result in a new Term, i.e. a Triangle that is MIN operated
  // with a value less than the apex, will convert to a trapezoid.
  // The xy vector<XY*> in the FuzzyOutputTerm activationTerm in the Rule
  // conclusion is used to hold this new term. 

  // Iterate through each output conclusion for this rule
  for ( conci = Conclusions.begin(); conci != Conclusions.end(); ++conci ) {
    lpConc = *conci;
    lpFOC  = lpConc->outputVariable;
    lpFOT  = lpConc->outputDefuzzify;
    lpFAT  = &(lpConc->activationTerm);

    DebugMsg("\tActivation for", lpFOC->varName + " IS " + 
	     lpFOT->termName, status);
    DebugMsg("\tAggregation value", conditionResult, status);

    // Clear out any pre-existing xy components in this term
    for ( xyi = lpFAT->xy.begin(); xyi != lpFAT->xy.end(); ++xyi ) {
      delete *xyi;
    }
    lpFAT->xy.clear();

    switch ( lpFOT->termType ) {

      case Trapezoid:
	lpFAT->termType = Trapezoid;

	if ( actMethod == kwd_Prod or conditionResult >= lpFOT->xy[1]->y ) {
	  // ACT is PROD or aggregation is > top of trapezoid, 
	  // just copy it, with multiply
	  for ( i = 0; i <= 3; i++ ) { 
	    // trapezoids have 4 points, vector stack is filo
	    lpXY = lpFOT->xy[i]; 
	    // create a new XY struct to hold the Activation term points
	    lpActXY = new XY;
	    if ( not lpActXY ) {
	      status = -1;
	      ErrMsg("Activate() Trapezoid Failed to create new XY", 
		     lpFOT->termName, status);
	      return status;
	    }
	    // Assign the new term points
	    lpActXY->x = lpXY->x;
	    if ( actMethod == kwd_Prod ) {
	      lpActXY->y = conditionResult * lpXY->y * lpConc->weight;
	    }
	    else {
	      lpActXY->y = lpXY->y * lpConc->weight;
	    }
	    if ( lpActXY->y < ZERO_TOLERANCE ) lpActXY->y = 0.;
	    // Assign the new XY struct to the conclusion activationTerm 
	    // xy vector
	    lpFAT->xy.push_back(lpActXY);
	  }
	}
	
	else if ( actMethod == kwd_Min ) {
	  // ACT = MIN

	  if ( conditionResult < lpFOT->xy[0]->y  ) {
	    // Aggregation < trapezoid low point
	    status = -1;
	    ErrMsg("Activate(): Activation is below term minimum for", 
		    lpFOC->varName + " IS " + lpFAT->termName, status);
	    return status;
	  }

	  // allocate containers for new Act trapezoid
	  lpActXY_0 = new XY; 
	  lpActXY_1 = new XY; 
	  lpActXY_2 = new XY; 
	  lpActXY_3 = new XY;
	  if ( not lpActXY_0 or not lpActXY_1 or 
	       not lpActXY_2 or not lpActXY_3 ) {
	    status = -1;
	    ErrMsg("Activate() Trapezoid Failed to create new XY", 
		   lpFOT->termName, status);
	    return status;
	  }

	  // First and fourth points stay the same
	  lpActXY_0->x = lpFOT->xy[0]->x;
	  lpActXY_0->y = lpFOT->xy[0]->y * lpConc->weight;
	  lpActXY_3->x = lpFOT->xy[3]->x;
	  lpActXY_3->y = lpFOT->xy[3]->y * lpConc->weight;
	  // Second and third points have y = aggregation value, x changes
	  lpActXY_1->y = conditionResult * lpConc->weight;
	  lpActXY_2->y = conditionResult * lpConc->weight;

	  // Find new x values for second & third points
	  if ( conditionResult > lpFOT->xy[0]->y and 
	       conditionResult > lpFOT->xy[2]->y and
	       lpFOT->xy[1]->y > lpFOT->xy[0]->y and 
	       lpFOT->xy[1]->x > lpFOT->xy[0]->x and
	       lpFOT->xy[3]->y > lpFOT->xy[2]->y and 
	       lpFOT->xy[3]->x > lpFOT->xy[2]->x) {

	    lpActXY_1->x = (conditionResult - lpFOT->xy[0]->y) /
	                    ( (lpFOT->xy[1]->y - lpFOT->xy[0]->y) /
	                      (lpFOT->xy[1]->x - lpFOT->xy[0]->x) );
	  
	    lpActXY_2->x = lpFOT->xy[2]->x + 
	                   ( conditionResult  - lpFOT->xy[2]->y ) /
	                    ( (lpFOT->xy[3]->y - lpFOT->xy[2]->y) /
	                      (lpFOT->xy[3]->x - lpFOT->xy[2]->x) );
	  }
	  else {
	    lpActXY_1->x = lpFOT->xy[1]->x;
	    lpActXY_2->x = lpFOT->xy[2]->x;
	  }
	  // sanity check
	  if ( lpActXY_0->y < ZERO_TOLERANCE ) lpActXY_0->y = 0.;
	  if ( lpActXY_1->y < ZERO_TOLERANCE ) lpActXY_1->y = 0.;
	  if ( lpActXY_2->y < ZERO_TOLERANCE ) lpActXY_2->y = 0.;
	  if ( lpActXY_3->y < ZERO_TOLERANCE ) lpActXY_3->y = 0.;

	  // Assign the new XY structs to the conclusion avtivationTerm 
	  // xy vector
	  lpFAT->xy.push_back(lpActXY_0);
	  lpFAT->xy.push_back(lpActXY_1);
	  lpFAT->xy.push_back(lpActXY_2);
	  lpFAT->xy.push_back(lpActXY_3);

	} // ACT = MIN

	DebugMsg("\tTerm Trapezoid output", lpFOC->varName + 
		 " IS " + lpFAT->termName, status);
	#ifdef DEBUG
	cout << "\t(" << lpFAT->xy[0]->x << ", " << lpFAT->xy[0]->y<< ")  ";
	cout << "("   << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y<< ")  ";
	cout << "("   << lpFAT->xy[2]->x << ", " << lpFAT->xy[2]->y<< ")  ";
	cout << "("   << lpFAT->xy[3]->x << ", " << lpFAT->xy[3]->y<< ")  " 
	     << "\n";
	#endif
	break;

      case Triangle:
	// A triangle can convert to a trapezoid if the aggregation value
	// is less than the apex.

	if ( actMethod == kwd_Prod or conditionResult >= lpFOT->xy[1]->y ) {
	  // ACT is PROD or aggregation is > apex of triangle, 
	  // just copy/multiply it
	  lpFAT->termType = Triangle;
	  for ( i = 0; i <= 2; i++ ) { // triangles have 3 points, 
	    // vector stack is filo
	    lpXY = lpFOT->xy[i];
	    // create a new XY struct to hold the Activation term points
	    lpActXY = new XY;
	    if ( not lpActXY ) {
	      status = -1;
	      ErrMsg("Activate() Triangle Failed to create new XY", 
		     lpFOT->termName, status);
	      return status;
	    }
	    // Assign the new term points
	    lpActXY->x = lpXY->x;
	    if ( actMethod == kwd_Prod ) {
	      lpActXY->y = conditionResult * lpXY->y * lpConc->weight;
	    }
	    else {
	      lpActXY->y = lpXY->y * lpConc->weight;
	    }
	    if ( lpActXY->y < ZERO_TOLERANCE ) lpActXY->y = 0.;
	    // Assign the new XY struct to the conclusion 
	    // avtivationTerm xy vector
	    lpFAT->xy.push_back(lpActXY);
	  }
	}

	else if ( actMethod == kwd_Min ) {
	  // ACT = MIN

	  if ( conditionResult < lpFOT->xy[0]->y  ) {
	    // Aggregation < triangle low point
	    status = -1;
	    ErrMsg("Activate(): Activation is below term minimum for", 
		    lpFOC->varName + " IS " + lpFAT->termName, status);
	    return status;
	  }

	  // Aggregation is less than apex of triangle, it becomes a trapezoid
	  // Even in the case where all of the y terms are zero, convert it to
	  // a trapezoid, as it makes accumulation easier.
	  lpFAT->termType = Trapezoid;

	  DebugMsg("\tConvert Triangle to Trapezoid", 
		   lpFOC->varName + " IS " + lpFAT->termName, status);

	  // allocate containers for new Act trapezoid
	  lpActXY_0 = new XY; 
	  lpActXY_1 = new XY; 
	  lpActXY_2 = new XY; 
	  lpActXY_3 = new XY;
	  if ( not lpActXY_0 or not lpActXY_1 or 
	       not lpActXY_2 or not lpActXY_3 ) {
	    status = -1;
	    ErrMsg("Activate() Triangle Failed to create new XY", 
		   lpFOT->termName, status);
	    return status;
	  }

	  // First and fourth points stay the same ordinates as 
	  // triangle first and third points
	  lpActXY_0->x = lpFOT->xy[0]->x;
	  lpActXY_0->y = lpFOT->xy[0]->y * lpConc->weight;
	  lpActXY_3->x = lpFOT->xy[2]->x;
	  lpActXY_3->y = lpFOT->xy[2]->y * lpConc->weight;
	  // Second and third points have y = aggregation value, x changes
	  lpActXY_1->y = conditionResult * lpConc->weight;
	  lpActXY_2->y = conditionResult * lpConc->weight;
	  if ( lpActXY_0->y < ZERO_TOLERANCE ) lpActXY_0->y = 0.;
	  if ( lpActXY_1->y < ZERO_TOLERANCE ) lpActXY_1->y = 0.;
	  if ( lpActXY_2->y < ZERO_TOLERANCE ) lpActXY_2->y = 0.;
	  if ( lpActXY_3->y < ZERO_TOLERANCE ) lpActXY_3->y = 0.;

	  if ( conditionResult >= lpFOT->xy[0]->y and
	       lpFOT->xy[1]->y >  lpFOT->xy[0]->y and 
	       lpFOT->xy[1]->x > lpFOT->xy[0]->x ) {
	    // Find new x values for second & third points
	    lpActXY_1->x = lpFOT->xy[0]->x +
	      (conditionResult - lpFOT->xy[0]->y) /
	      ( (lpFOT->xy[1]->y - lpFOT->xy[0]->y) /
		(lpFOT->xy[1]->x - lpFOT->xy[0]->x) );
	  
	    lpActXY_2->x = lpFOT->xy[1]->x + 
	      ( conditionResult  - lpFOT->xy[1]->y ) /
	      ( (lpFOT->xy[2]->y - lpFOT->xy[1]->y) /
		(lpFOT->xy[2]->x - lpFOT->xy[1]->x) );
	  }
	  else {
	    // halfway between the triangle points
	    lpActXY_1->x = lpFOT->xy[0]->x + 
	                   (lpFOT->xy[1]->x - lpFOT->xy[0]->x)/2.;
	    lpActXY_2->x = lpFOT->xy[1]->x + 
	                   (lpFOT->xy[2]->x - lpFOT->xy[1]->x)/2.;
	  }
	  // In case the generated trapezoid has first and second ordinates 
	  // the same,  or third and fourth ordinates the same, which will 
	  // happen if the y terms are all 0, set midway values so that 
	  // the accumulation routine won't complain.
	  if (lpActXY_0->x == lpActXY_1->x) {
	    lpActXY_1->x = lpFOT->xy[0]->x + 
	                   (lpFOT->xy[1]->x - lpFOT->xy[0]->x)/2.;
	  }
	  if (lpActXY_2->x == lpActXY_3->x) {
	    lpActXY_2->x = lpFOT->xy[1]->x + 
	                   (lpFOT->xy[2]->x - lpFOT->xy[1]->x)/2.;
	  }
	  // Assign the new XY structs to the conclusion activationTerm 
	  // xy vector
	  lpFAT->xy.push_back(lpActXY_0);
	  lpFAT->xy.push_back(lpActXY_1);
	  lpFAT->xy.push_back(lpActXY_2);
	  lpFAT->xy.push_back(lpActXY_3);
	}

	DebugMsg("\tTerm Triangle output", lpFOC->varName + 
		 " IS " + lpFAT->termName, status);
	#ifdef DEBUG 
	cout << "\t(" << lpFAT->xy[0]->x << ", " << lpFAT->xy[0]->y<< ")  ";
	cout << "("   << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y<< ")  ";
	cout << "("   << lpFAT->xy[2]->x << ", " << lpFAT->xy[2]->y<< ")  ";
	if ( lpFAT->termType == Trapezoid )
	  cout << "(" << lpFAT->xy[3]->x << ", " << lpFAT->xy[3]->y<< ")  ";
	cout << "\n";
	#endif
	break;

      case Ramp:
	// Ramps will stay as Ramps, but the x,y points may move
	lpFAT->termType = Ramp;

	rampUp   = false;
	rampDown = false;
	aggregationAbove = false;
	aggregationBelow = false;

	// Classify the term for easier handling below
	if ( lpFOT->xy[1]->y > lpFOT->xy[0]->y ) {
	  rampUp = true;
	  if ( conditionResult >= lpFOT->xy[1]->y ) aggregationAbove = true;
	  if ( conditionResult <  lpFOT->xy[0]->y ) aggregationBelow = true;
	}
	if ( lpFOT->xy[0]->y > lpFOT->xy[1]->y ) {
	  rampDown = true;
	  if ( conditionResult >= lpFOT->xy[0]->y ) aggregationAbove = true;
	  if ( conditionResult <  lpFOT->xy[1]->y ) aggregationBelow = true;
	}
	if ( (rampUp and rampDown) or 
	     (aggregationAbove and aggregationBelow) ) {
	  status = -1;
	  ErrMsg("Activate() Invalid term configuration", 
		 lpFOT->termName, status);
	  return status;
	}

	// Check for cases that are degenerate between up/down ramps
	if ( actMethod == kwd_Prod or aggregationAbove ) { 
	  // ACT is PROD or aggregation result > max of ramp
	  // Ramp will not change 
	  for ( i = 0; i <= 1; i++ ) { 
	    // ramps have 2 points, vector stack is filo
	    lpXY = lpFOT->xy[i];
	    // create a new XY struct to hold the Activation term points
	    lpActXY = new XY;
	    if ( not lpActXY ) {
	      status = -1;
	      ErrMsg("Activate() Ramp Failed to create new XY", 
		     lpFAT->termName, status);
	      return status;
	    }
	    // Assign the new term points
	    lpActXY->x = lpXY->x;
	    if ( actMethod == kwd_Prod ) {
	      lpActXY->y = conditionResult * lpXY->y * lpConc->weight;
	    }
	    else {
	      lpActXY->y = lpXY->y * lpConc->weight;
	    }
	    if ( lpActXY->y < ZERO_TOLERANCE ) lpActXY->y = 0.;
	    // Assign the new XY struct to the FuzzyOutputTerm xyAct vector
	    lpFAT->xy.push_back(lpActXY);
	  }
	}

	else if ( actMethod == kwd_Min ) {
	  // ACT = MIN
	  if ( aggregationBelow ) {
	    // Aggregation < ramp low point, it becomes a singleton?
	    status = -1;
	    ErrMsg("Activate(): Activation is below term minimum for", 
		    lpFOC->varName + " IS " + lpFAT->termName, status);
	    return status;
	  }

	  else if ( rampUp ) {
	    // Aggregation is between ramp low/high
	    // Find new ordinate, abcissa values
	    lpActXY_0 = new XY; lpActXY_1 = new XY;
	    if ( not lpActXY_0 or not lpActXY_1 ) {
	      status = -1;
	      ErrMsg("Activate() Ramp Failed to create new XY", 
		     lpFAT->termName, status);
	      return status;
	    }
	    // Assign the new term points
	    // Lower point doesn't change
	    lpActXY_0->x = lpFOT->xy[0]->x;
	    lpActXY_0->y = lpFOT->xy[0]->y * lpConc->weight;

	    if ( conditionResult > lpFOT->xy[0]->y and
		 lpFOT->xy[1]->y > lpFOT->xy[0]->y and 
		 lpFOT->xy[1]->x > lpFOT->xy[0]->x) {
	      // Second point changes x, y = aggregation value
	      lpActXY_1->x = (conditionResult - lpFOT->xy[0]->y) /
	                      ( (lpFOT->xy[1]->y - lpFOT->xy[0]->y) /
		                (lpFOT->xy[1]->x - lpFOT->xy[0]->x) );
	      // ensure the new x oridnate is at least past the first ordinate
	      if ( lpActXY_1->x < lpFOT->xy[0]->x ) {
		lpActXY_1->x = lpFOT->xy[1]->x;
	      }
	    }
	    else {
	      lpActXY_1->x = lpFOT->xy[1]->x;
	    }

	    lpActXY_1->y = conditionResult * lpConc->weight;		  

	    if ( lpActXY_0->y < ZERO_TOLERANCE ) lpActXY_0->y = 0.;
	    if ( lpActXY_1->y < ZERO_TOLERANCE ) lpActXY_1->y = 0.;
	    // Assign the new XY struct to the FuzzyOutputTerm xyAct vector
	    lpFAT->xy.push_back(lpActXY_0);
	    lpFAT->xy.push_back(lpActXY_1);
	  } // Ramp is 'up'

	  else if ( rampDown ) {
	    // Aggregation is between ramp low/high
	    // Find new ordinate, abcissa values
	    lpActXY_0 = new XY; lpActXY_1 = new XY;
	    if ( not lpActXY_0 or not lpActXY_1 ) {
	      status = -1;
	      ErrMsg("Activate() Ramp Failed to create new XY", 
		     lpFAT->termName, status);
	      return status;
	    }
	    // Assign the new term points
	    // First point changes x, y = aggregation value
	    if ( conditionResult > lpFOT->xy[0]->y and
		 lpFOT->xy[1]->y > lpFOT->xy[0]->y and 
		 lpFOT->xy[1]->x > lpFOT->xy[0]->x) {
	      lpActXY_0->x = (conditionResult - lpFOT->xy[0]->y) /
	                      ( (lpFOT->xy[1]->y - lpFOT->xy[0]->y) /
		                (lpFOT->xy[1]->x - lpFOT->xy[0]->x) );

	      // ensure the new x oridnate is not past the second ordinate
	      if ( lpActXY_0->x > lpFOT->xy[1]->x ) {
		lpActXY_0->x = lpFOT->xy[0]->x;
	      }
	    }
	    else {
	      lpActXY_0->x = lpFOT->xy[0]->x;
	    }
	    lpActXY_0->y = conditionResult * lpConc->weight;		  
	    // Lower point doesn't change
	    lpActXY_1->x = lpFOT->xy[1]->x;
	    lpActXY_1->y = lpFOT->xy[1]->y * lpConc->weight;
	    if ( lpActXY_0->y < ZERO_TOLERANCE ) lpActXY_0->y = 0.;
	    if ( lpActXY_1->y < ZERO_TOLERANCE ) lpActXY_1->y = 0.;
	    // Assign the new XY struct to the FuzzyOutputTerm xyAct vector
	    lpFAT->xy.push_back(lpActXY_0);
	    lpFAT->xy.push_back(lpActXY_1);
	  }
	} // ACT = MIN

	DebugMsg("\tTerm Ramp output", lpFOC->varName + 
		 " IS " + lpFAT->termName, status);
	#ifdef DEBUG 
	cout << "\t(" << lpFAT->xy[0]->x << ", " << lpFAT->xy[0]->y<< ")  ";
	cout << "("   << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y<< ")\n";
	#endif
	break;
	
      case Rectangle:

	if ( actMethod == kwd_Min and conditionResult < lpFOT->xy[0]->y  ) {
	  // Aggregation < rectangle low point
	  status = -1;
	  ErrMsg("Activate(): Activation is below term minimum for", 
		 lpFOC->varName + " IS " + lpFAT->termName, status);
	  return status;
	}

	// Rectangles are just abcissa scaled, the ordinates remain the same
	lpFAT->termType = Rectangle;

	// Iterate though each point of the FCL defined term in vector<XY*> xy
	for ( xyi = lpFOT->xy.begin(); xyi != lpFOT->xy.end(); ++xyi ) {
	  // create a new XY struct to hold the Activation term points
	  lpXY = *xyi;
	  lpActXY = new XY;
	  if ( not lpActXY ) {
	    ErrMsg("Activate() Rectangle Failed to create new XY", 
		   lpFOT->termName, status);
	  }
	  // Assign the ordinates & scaled abcissa to the new term point
	  lpActXY->x = lpXY->x;
	  if ( actMethod == kwd_Min ) {
	    lpActXY->y = min(conditionResult, lpXY->y) * lpConc->weight;
	  }
	  else if ( actMethod == kwd_Prod ) {
	    lpActXY->y = conditionResult * lpXY->y * lpConc->weight;
	  }
	  // Assign the new XY struct to the FuzzyOutputTerm xyAct vector
	  lpFAT->xy.push_back(lpActXY);
	}
	DebugMsg("\tTerm Rectangle output", lpFOC->varName + 
		 " IS " + lpFAT->termName, status);
	#ifdef DEBUG
	cout << "\t(" << lpFAT->xy[0]->x << ", " << lpFAT->xy[0]->y<< ")  ";
	cout << "("   << lpFAT->xy[1]->x << ", " << lpFAT->xy[1]->y<< ")  ";
	cout << "("   << lpFAT->xy[2]->x << ", " << lpFAT->xy[2]->y<< ")  ";
	cout << "("   << lpFAT->xy[3]->x << ", " << lpFAT->xy[3]->y<< ")  " 
	     << "\n";
	#endif
	break;

      case Singleton:
	// If the output term is a singleton, then the aggregation value is used
	lpFAT->termType    = Singleton;
	lpFAT->singleton.y = conditionResult * lpConc->weight;
	lpFAT->singleton.x = lpFOT->singleton.x;

	DebugMsg("\tTerm singleton output", lpFOC->varName + 
		 " IS " + lpFAT->termName, status);
	#ifdef DEBUG
	cout << "\t(" << lpFAT->singleton.x << ", " 
	     << lpFAT->singleton.y << ")\n";
	#endif
	break;

      default:
	status = -1;
	ErrMsg("Activate() Invalid term type", lpFOT->termName, status);
	return status;

    }; // switch ( lpFOT->termType ) 
  
  } // Iterate through each output conclusion for this rule

  return status;
}

//--------------------------------------------------------------
// AND_SubConditions
//
// Purpose: Perform the appropriate AND operation
//          Valid operations are: MIN, PROD, BDIF
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyRuleClass::AND_SubConditions() {

  int status = 0;

  vector<Condition*>    :: iterator condi;      // Conditions
  vector<SubCondition*> :: iterator andSubCondi;

  Condition*       lpCond;       // pntr to Fuzzy Rule Condition struct
  SubCondition*    lpANDSubCond; // pntr to Condition AND_SubConditions list
  FuzzyInputClass* lpFIC;        // pntr to SubCondition Fuzzy Input
  FuzzyInputTerm*  lpFIT;        // pntr to SubCondition Fuzzy Term

  // Iterate through the Conditions vector of this rule to find 
  // the subCondition results
  for ( condi = Conditions.begin(); condi != Conditions.end(); ++condi ) {
    double lastTermMembership = 1.;
    double thisTermMembership = 0.;
    double subResult = 1.;
    lpCond = *condi;

    if ( not lpCond->AND_SubConditions.size() ) {
      continue;
    }

    // Iterate through the SubConditions of this condition
    // The result is a value assigned to the SubCondition.subResult
    for ( andSubCondi =  lpCond->AND_SubConditions.begin(); 
	  andSubCondi != lpCond->AND_SubConditions.end(); ++andSubCondi ) {
      lpANDSubCond = *andSubCondi;
      lpFIC = lpANDSubCond->inputVariable;
      lpFIT = lpANDSubCond->inputFuzzifyTerm;

      // If the term is prefixed with NOT, take the compliment
      if ( lpANDSubCond->notTerm ) {
	thisTermMembership = 1. - lpFIT->membership;
	DebugAllMsg("\tAND", lpFIC->varName + " IS NOT " + lpFIT->termName, 0);
      }
      else if ( lpANDSubCond->notCondition ) {
	thisTermMembership = lpFIT->membership;
	DebugAllMsg("\tAND  NOT ", "( " + lpFIC->varName + 
		    " IS " + lpFIT->termName + " )", 0);
      }
      else {
	thisTermMembership = lpFIT->membership;
	DebugAllMsg("\tAND", lpFIC->varName + " IS " + lpFIT->termName, 0);
      }

      DebugAllMsg("\tThisTerm: ", thisTermMembership, status);
      DebugAllMsg("\tLastTerm: ", lastTermMembership, status);

      // If there is only one subCondition, then return it's value
      if ( lpCond->AND_SubConditions.size() == 1 ) {
	subResult = lpFIT->membership;
	DebugAllMsg("\tInterm subResult Only 1 SubCondition", subResult, 0);
	continue;
      }

      // Find the SubCondition subResult by combining the terms with andMethod
      if ( andMethod == kwd_Min ) {
	// AND = MIN = Min(u1, u2)
	// lastTermMembership was set to 1 for the initial iteration
	subResult = min(lastTermMembership, thisTermMembership);
      }
      else if ( andMethod == kwd_Prod ) {
	// AND = PROD = u1 * u2
	// subResult was set to 1 for the initial iteration
	subResult *= thisTermMembership;
      }
      else if ( andMethod == kwd_Bdif ) {
	// AND = BDIF = Max( 0, u1 + u2 - 1 )
	// on first iteration, lastTermMembership = 1
	subResult = max( 0., lastTermMembership + thisTermMembership - 1 );
      }
      else {
	status = -1;
	ErrMsg("Failed to find valid AND method for subCondition",
	       lpFIC->varName + " IS " + lpFIT->termName, status);
	return status;
      }
      lastTermMembership = thisTermMembership;

      DebugAllMsg("\tInterm subResult", subResult, 0);

    } // Iterate through the SubConditions of this condition

    // Assign the SubCondition.subResult
    // If the SubCondition is prefixed with NOT, take the compliment
    if ( lpANDSubCond->notCondition ) {
      subResult = 1. - subResult;
    }
    lpANDSubCond->subResult = subResult;

    // Assign the rule Condition result
    lpCond->result  = subResult;
    conditionResult = subResult;
    DebugAllMsg("\tAND Final result", lpCond->result, 0);

  } // Iterate through the Conditions vector of this rule

  return status;
}

//--------------------------------------------------------------
// OR_SubConditions
//
// Purpose: Perform the appropriate OR operation
//          Valid operations are: MAX, ASUM, BSUM
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyRuleClass::OR_SubConditions() {

  int status = 0;

  vector<Condition*>    :: iterator condi;      // Conditions
  vector<SubCondition*> :: iterator orSubCondi;

  Condition*       lpCond;       // pntr to Fuzzy Rule Condition struct
  SubCondition*    lpORSubCond;  // pntr to Condition OR_SubConditions list
  FuzzyInputClass* lpFIC;        // pntr to SubCondition Fuzzy Input
  FuzzyInputTerm*  lpFIT;        // pntr to SubCondition Fuzzy Term

  // Iterate through the Conditions vector of this rule to 
  // find the subCondition results
  for ( condi = Conditions.begin(); condi != Conditions.end(); ++condi ) {
    double lastTermMembership = 0.;
    double thisTermMembership = 0.;
    double subResult = 0.;
    lpCond = *condi;

    if ( not lpCond->OR_SubConditions.size() ) {
      continue;
    }

    // Iterate through the SubConditions of this condition
    // The result is a value assigned to the SubCondition.subResult
    for ( orSubCondi =  lpCond->OR_SubConditions.begin(); 
	  orSubCondi != lpCond->OR_SubConditions.end(); ++orSubCondi ) {
      lpORSubCond = *orSubCondi;
      lpFIC = lpORSubCond->inputVariable;
      lpFIT = lpORSubCond->inputFuzzifyTerm;

      // If the term is prefixed with NOT, take the compliment
      if ( lpORSubCond->notTerm ) {
	thisTermMembership = 1. - lpFIT->membership;
	DebugAllMsg("\tOR", lpFIC->varName + " IS NOT " + lpFIT->termName, 0);
      }
      else if ( lpORSubCond->notCondition ) {
	thisTermMembership = lpFIT->membership;
	DebugAllMsg("\tOR  NOT ", "( " + lpFIC->varName + 
		    " IS " + lpFIT->termName + " )", 0);
      }
      else {
	thisTermMembership = lpFIT->membership;
	DebugAllMsg("\tOR", lpFIC->varName + " IS " + lpFIT->termName, 0);
      }

      DebugAllMsg("\tThisTerm: ", thisTermMembership, status);
      DebugAllMsg("\tLastTerm: ", lastTermMembership, status);

      // If there is only one subCondition, then return it's value
      if ( lpCond->OR_SubConditions.size() == 1 ) {
	subResult = lpFIT->membership;
	DebugAllMsg("\tInterm subResult Only 1 SubCondition", subResult, 0);
	continue;
      }

      // Find the SubCondition subResult by combining the terms with orMethod
      if ( orMethod == kwd_Max ) {
	// OR = MAX = Max(u1, u2)
	// lastTermMembership was set to 0 for the initial iteration
	subResult = max(lastTermMembership, thisTermMembership);
      }
      else if ( orMethod == kwd_Asum ) {
	// OR = ASUM = u1 + u2 - (u1 * u2)
	// lastTermMembership was set to 0 for the initial iteration
	subResult = lastTermMembership + thisTermMembership - 
	           (lastTermMembership * thisTermMembership);
      }
      else if ( orMethod == kwd_Bsum ) {
	// OR = BSUM = Min( 1, u1 + u2 )
	// lastTermMembership was set to 0 for the initial iteration
	subResult = min(1., lastTermMembership + thisTermMembership);
      }
      else {
	status = -1;
	ErrMsg("Failed to find valid OR method for subCondition",
	       lpFIC->varName + " IS " + lpFIT->termName, status);
	return status;
      }
      lastTermMembership = thisTermMembership;

      DebugAllMsg("\tInterm subResult", subResult, 0);

    } // Iterate through the SubConditions of this condition

    // Assign the SubCondition.subResult
    // If the SubCondition is prefixed with NOT, take the compliment
    if ( lpORSubCond->notCondition ) {
      subResult = 1. - subResult;
    }
    lpORSubCond->subResult = subResult;

    // Assign the rule Condition result
    lpCond->result  = subResult;
    conditionResult = subResult;
    DebugAllMsg("\tOR Final result", lpCond->result, 0);

  } // Iterate through the Conditions vector of this rule

  return status;
}

//--------------------------------------------------------------
// FuzzyRuleClass
//
// Purpose: Constructor for FuzzyRuleClass
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
FuzzyRuleClass::FuzzyRuleClass(FCL_keyword* kwdMin,  FCL_keyword* kwdMax, 
			       FCL_keyword* kwdProd, FCL_keyword* kwdBdif,
			       FCL_keyword* kwdAsum, FCL_keyword* kwdBsum) {
  
  // Set default values
  andMethod = kwdMin;
  orMethod  = kwdMax;
  actMethod = kwdMin;

  // Save access pointers to keywords needed for AND/OR methods
  kwd_Min  = kwdMin;
  kwd_Max  = kwdMax;
  kwd_Prod = kwdProd;
  kwd_Bdif = kwdBdif;
  kwd_Asum = kwdAsum;
  kwd_Bsum = kwdBsum;

  nSubConclusions = 0;
  nSubConditions  = 0;
}
