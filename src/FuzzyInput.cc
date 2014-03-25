#include "FuzzyControl.h"

//--------------------------------------------------------------
// FuzzyInputClass
//
// Purpose: Constructor for FuzzyInputClass
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
FuzzyInputClass::FuzzyInputClass() {

}

//--------------------------------------------------------------
// FuzzifyTriangle
//
// Purpose: 
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyInputClass::FuzzifyTriangle(FuzzyInputTerm* fit, double inputValue) {

  int status = 0;

  // set membership to zero
  fit->membership = 0.;

  // There are three points in the term (membership function): 
  // (x0, y0), (x1, y1), (x2, y2)

  // Each point is an XY struct in the vector xy
  vector<XY*> lpXY = fit->xy;

  // Perform a linear interpolation of the form: y = y0 + mx
  // where y0 is the reference datum, m the slope, and x the argument

  if ( inputValue <= lpXY[0]->x ) {
    // if the inputValue is below the range of the triangle, return low
    fit->membership = lpXY[0]->y;
  }
  else if ( inputValue >= lpXY[2]->x ) {
    // if the inputValue is above the range of the triangle, return high
    fit->membership = lpXY[2]->y;
  }
  else if ( inputValue <= lpXY[1]->x ) {
    // else if input is within triangle 'up ramp'
    if (lpXY[1]->x <= lpXY[0]->x) {
      status = -1;
      ErrMsg("FuzzifyTriangle() Invalid ordinates", fit->termName, status);
      return status;
    }
    fit->membership = lpXY[0]->y + 
                      ( (lpXY[1]->y - lpXY[0]->y) / 
                        (lpXY[1]->x - lpXY[0]->x) ) *
                      ( inputValue - lpXY[0]->x );
  }
  else if ( inputValue > lpXY[1]->x ) {
    // else if input is within triangle 'down ramp'
    if (lpXY[2]->x <= lpXY[1]->x) {
      status = -1;
      ErrMsg("FuzzifyTriangle() Invalid ordinates", fit->termName, status);
      return status;
    }
    fit->membership = lpXY[1]->y - 
                      ( (lpXY[1]->y - lpXY[2]->y) / 
                        (lpXY[2]->x - lpXY[1]->x) ) *
                      ( inputValue - lpXY[1]->x );
  }
  else {
    status = -1;
    ErrMsg("FuzzifyTriangle():: Invalid inputValue range for term", 
	   fit->termName, status);
    return status;
  }

  if ( fit->membership < ZERO_TOLERANCE ) fit->membership = 0.;

  DebugMsg("\tFuzzifyTriangle():: Assigned membership", fit->membership, 0);

  return status;
}

//--------------------------------------------------------------
// FuzzifyRamp
//
// Purpose:
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyInputClass::FuzzifyRamp(FuzzyInputTerm* fit, double inputValue) {

  int status = 0;

  // set membership to zero
  fit->membership = 0.;

  // There are two points in the term (membership function): 
  // (x0, y0), (x1, y1)

  // Each point is an XY struct in the vector xy
  vector<XY*> lpXY = fit->xy;

  // Perform a linear interpolation of the form: y = y0 + mx
  // where y0 is the reference datum, m the slope, and x the argument

  if ( inputValue <= lpXY[0]->x ) {
    // if the inputValue is below the range of the ramp, return low
    fit->membership = lpXY[0]->y;
  }
  else if ( inputValue >= lpXY[1]->x ) {
    // if the inputValue is above the range of the ramp, return high
    fit->membership = lpXY[1]->y;
  }
  else if ( inputValue < lpXY[1]->x ) {
    if (lpXY[1]->x <= lpXY[0]->x) {
      status = -1;
      ErrMsg("FuzzifyRamp() Invalid ordinates", fit->termName, status);
      return status;
    }
    if ( lpXY[0]->y < lpXY[1]->y ) {
      // the ramp is rising
      fit->membership = lpXY[0]->y + 
	                ( (lpXY[1]->y - lpXY[0]->y) / 
                          (lpXY[1]->x - lpXY[0]->x) ) *
	                ( inputValue - lpXY[0]->x );
    }
    else {
      // the ramp is falling
      fit->membership = lpXY[0]->y - 
	                ( (lpXY[0]->y - lpXY[1]->y) / 
                          (lpXY[1]->x - lpXY[0]->x) ) *
	                ( inputValue - lpXY[0]->x );
    }
  }
  else {
    status = -1;
    ErrMsg("FuzzifyRamp():: Invalid inputValue range for term", 
	   fit->termName, status);
    return status;
  }

  if ( fit->membership < ZERO_TOLERANCE ) fit->membership = 0.;

  DebugMsg("\tFuzzifyRamp():: Assigned membership", fit->membership, 0);

  return status;
}

//--------------------------------------------------------------
// FuzzifyTrapezoid
//
// Purpose:
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyInputClass::FuzzifyTrapezoid(FuzzyInputTerm* fit, double inputValue) {

  int status = 0;

  // set membership to zero
  fit->membership = 0.;

  // There are four points in the term (membership function): 
  // (x0, y0), (x1, y1), (x2, y2), (x3, y3)
  // the ordinates of the 1st & 2nd, and 3rd & 4th points are not the same

  // Each point is an XY struct in the vector xy
  vector<XY*> lpXY = fit->xy;

  // Perform a linear interpolation of the form: y = y0 + mx
  // where y0 is the reference datum, m the slope, and x the argument

  if ( inputValue <= lpXY[0]->x ) {
    // if the inputValue is below the range of the trapezoid, return low
    fit->membership = lpXY[0]->y;
  }
  else if ( inputValue >= lpXY[3]->x ) {
    // if the inputValue is above the range of the trapezoid, return high
    fit->membership = lpXY[3]->y;
  }
  else if ( inputValue <= lpXY[1]->x ) {
    // the rising ramp section
    if (lpXY[1]->x <= lpXY[0]->x) {
      status = -1;
      ErrMsg("FuzzifyTrapezoid() Invalid ordinates", fit->termName, status);
      return status;
    }
    fit->membership = lpXY[0]->y + 
                      ( (lpXY[1]->y - lpXY[0]->y) / 
                        (lpXY[1]->x - lpXY[0]->x) ) *
                      ( inputValue - lpXY[0]->x );
  }
  else if ( inputValue <= lpXY[2]->x ) {
    // flat top of trapezoid section
    fit->membership = lpXY[1]->y;
  }
  else if ( inputValue < lpXY[3]->x ) {
    // the falling ramp section
    if (lpXY[3]->x <= lpXY[2]->x) {
      status = -1;
      ErrMsg("FuzzifyTrapezoid() Invalid ordinates", fit->termName, status);
      return status;
    }
    fit->membership = lpXY[2]->y - 
                      ( (lpXY[2]->y - lpXY[3]->y) / 
                        (lpXY[3]->x - lpXY[2]->x) ) *
                      ( inputValue - lpXY[2]->x );
  }
  else {
    status = -1;
    ErrMsg("FuzzifyTrapezoid() Invalid inputValue range for term", 
	   fit->termName, status);
    return status;
  }

  if ( fit->membership < ZERO_TOLERANCE ) fit->membership = 0.;

  DebugMsg("\tFuzzifyTrapezoid():: Assigned membership", fit->membership, 0);

  return status;
}

//--------------------------------------------------------------
// FuzzifyRectangle
//
// Purpose:
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyInputClass::FuzzifyRectangle(FuzzyInputTerm* fit, double inputValue) {

  int status = 0;

  // set membership to zero
  fit->membership = 0.;

  // There are four points in the term (membership function): 
  // (x0, y0), (x1, y1), (x2, y2), (x3, y3)
  // the ordinates of the 1st & 2nd, and 3rd & 4th points are the same

  // Each point is an XY struct in the vector xy
  vector<XY*> lpXY = fit->xy;

  if ( inputValue < lpXY[0]->x ) {
    // if the inputValue is below the range of the rectangle, return low
    fit->membership = lpXY[0]->y;
  }
  else if ( inputValue > lpXY[3]->x ) {
    // if the inputValue is above the range of the rectangle, return high
    fit->membership = lpXY[3]->y;
  }
  else if ( inputValue >= lpXY[1]->x and inputValue <= lpXY[3]->x ) {
    // flat top of rectangle section
    fit->membership = lpXY[1]->y;
  }
  else {
    status = -1;
    ErrMsg("FuzzifyRectangle():: Invalid inputValue range for term", 
	   fit->termName, status);
    return status;
  }

  if ( fit->membership < ZERO_TOLERANCE ) fit->membership = 0.;

  DebugMsg("\tFuzzifyRectangle():: Assigned membership", fit->membership, 0);

  return status;
}

//--------------------------------------------------------------
// FuzzifySingleton
//
// Purpose:
//
// Arguments: 
//           
// Return:   
//--------------------------------------------------------------
int FuzzyInputClass::FuzzifySingleton(FuzzyInputTerm* fit, double inputValue) {

  int status = 0;

  // The point is an XY struct in the vector xy
  vector<XY*> lpXY = fit->xy;

  // Only one point in term, use it if the oridnate matches
  if ( inputValue == lpXY[0]->x ) {
    fit->membership = lpXY[0]->y;
  }
  else {
    fit->membership = 0.0;
  }

  DebugAllMsg("\tFuzzifySingleton():: Assigned membership", fit->membership, 0);

  return status;
}
