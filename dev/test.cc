// 
// test.cc
// 
// test FCL functions
//

#include "FuzzyControl.h"

int main(int argc, char* argv[])
{
  int status = 0;
  string FCLFileName;
  string inputDataFileName;
  string outputDataFileName;

  DebugMsg("->", "main()", status);

  // Get the FCL file name
  if (argc < 2) {
    DebugMsg("No input files specified, use test.fcl, test.in, test.out", "", status);
    FCLFileName        = "test.fcl";
    inputDataFileName  = "test.in";
    outputDataFileName = "test.out";
  }
  else {
    if ( argc < 3 ) {
      status = -1;
      DebugMsg("Invalid arguments, usage", "main fcl_file input_data_file [output_data_file]", status);
      return status;  
    }
    FCLFileName        = argv[1];
    inputDataFileName  = argv[2];
    if ( argc > 3 ) {
      outputDataFileName = argv[3];
    }
  }

  // Instantiate the main FuzzyControlClass
  // The FuzzyControlClass constructor will also instantiate 
  // the FLC_IO_Class which will initialize keywords map
  FuzzyControlClass FuzzyControl(FCLFileName);

  // Open the FCL file, buffer contents, close file
  status = FuzzyControl.ReadFCLFile();
  if ( status != 0 ) {
    ErrMsg( "Failed to read FCL file", FCLFileName, status );
    return status;
  }

  // Parse the FCL file, create FuzzyControl variables
  status = FuzzyControl.ParseFCLFile();
  if ( status != 0 ) {
    ErrMsg( "Failed to parse FCL file", FCLFileName, status );
    return status;
  }

  // Read in the fuzzy data input from a file
  // Buffer into the FuzzyControl::InputData map
  int numDataPoints = 0;
  numDataPoints = FuzzyControl.ReadInputDataFile( &inputDataFileName );
  DebugMsg("Read in data points", numDataPoints, 0);
  if ( numDataPoints < 0 ) {
    ErrMsg( "Failed to read input data file", inputDataFileName, numDataPoints );
    return numDataPoints;
  }
  // Open the output data file if requested
  if ( not outputDataFileName.empty() ) {
    status = FuzzyControl.OpenOutputFile( &outputDataFileName );
    if ( status != 0 ) {
      return status;
    }
  }

  // Process each input data value, ReadInputDataFile() returns numDataPoints
  for ( int i = 0; i < numDataPoints; i++ ) {
    // for each point in the InputData vector (timeseries)
    DebugMsg(">>>> Processing input data set", i+1, status);

    // STEP 1: FUZZIFICATION
    // Conversion of input values to linguistic variables
    // Iterate through the InputData variables
    status = FuzzyControl.Fuzzification(i);
    if ( status != 0 ) {
      ErrMsg( "Fuzzification Failed.", "", status );
      break;
    }

    // INFERENCE
    // Identify which rules apply, and compute output linguistic variables

    // STEP 2: INFERENCE: AGGREGATION
    // Determine degree of conformance of the rule conditions from the degree of
    // membership of the condition terms, i.e. consolodate the conditions into
    // a single membership value based on AND/OR of sub-conditions.
    // Method: for each rule, find the AND (Min) or OR (Max) combination of rule
    // conditions to result in a single condition (premise) membership number.
    // The AND/OR operators are defined in the RULEBLOCK, and have a keyword 
    // identifier in the FuzzyRuleClass objects andMethod, orMethod
    status = FuzzyControl.Aggregation();
    if ( status != 0 ) {
      ErrMsg("Aggregation Failed.", "", status);
      break;
    }

    // STEP 3: INFERENCE: ACTIVATION
    // Activation (assign the value) of the IF-THEN conclusion
    // For each rule, the result of the condition aggregation is convolved through
    // the output term(s) with the ACT operator (PROD or MIN).
    // This produces a modified FuzzyOutputTerm. The resulting FuzzyOutputTerm
    // is the activationTerm struct in the rule Conclusion struct.
    // Iterate through the rule base
    status = FuzzyControl.Activation();
    if ( status != 0 ) {
      ErrMsg("Activation Failed.", "", status);
      break;
    }

    // STEP 4: INFERENCE: ACCUMULATION
    // Combination of the weighted results of the rules into an overall result
    // Accumulation records the MAX, ASUM or BSUM accumulation of each output
    // term over all the rules. The resulting FuzzyOutputTerm accumulationTerm
    // is a struct in the FuzzyOutputClass.
    status = FuzzyControl.Accumulation();
    if ( status != 0 ) {
      ErrMsg("Accumulation Failed.", "", status);
      break;
    }
    
    // STEP 5: DEFUZZIFICATION
    // Conversion of linguistic output variables into crisp values
    status = FuzzyControl.Defuzzification();
    if ( status != 0 ) {
      ErrMsg("Defuzzification Failed.", "", status);
      break;
    }

    // Write output data if requested
    if ( not outputDataFileName.empty() ) {
      status = FuzzyControl.WriteOutputFile( i );
      if ( status != 0 ) {
	ErrMsg("WriteOutputFile() Failed.", outputDataFileName, status);
	break;
      }
    }

  } // for ( int i = 0; i < numDataPoints; i++ )
  
  // Close the output data file if requested
  if ( not outputDataFileName.empty() ) {
    status = FuzzyControl.CloseOutputFile();
    if ( status != 0 ) {
      ErrMsg("CloseOutputFile() Failed.", outputDataFileName, status);
    }
  }

  DebugMsg("<-", "main()", status);
}

///////////////////////////////////////////////////////////////////////////////////
