
#include "FuzzyControl.h"

//--------------------------------------------------------------
// FuzzyControl_SingleInput
//
// Purpose: Run control on single input point
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_SingleInput(FuzzyControlClass* lpFC) {

  int status = 0;

  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_SingleInput()", "Invalid FuzzyControlClass", status);
    return status;
  }

  // STEP 1: FUZZIFICATION
  // Conversion of input values to linguistic variables
  // Must be performed prior to calling this function

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
  status = lpFC->Aggregation();
  if ( status != 0 ) {
    ErrMsg("Aggregation Failed.", "", status);
    return status;
  }

  // STEP 3: INFERENCE: ACTIVATION
  // Activation (assign the value) of the IF-THEN conclusion
  // For each rule, the result of the condition aggregation is convolved through
  // the output term(s) with the ACT operator (PROD or MIN).
  // This produces a modified FuzzyOutputTerm. The resulting FuzzyOutputTerm
  // is the activationTerm struct in the rule Conclusion struct.
  // Iterate through the rule base
  status = lpFC->Activation();
  if ( status != 0 ) {
    ErrMsg("Activation Failed.", "", status);
    return status;
  }

  // STEP 4: INFERENCE: ACCUMULATION
  // Combination of the weighted results of the rules into an overall result
  // Accumulation records the MAX, ASUM or BSUM accumulation of each output
  // term over all the rules. The resulting FuzzyOutputTerm accumulationTerm
  // is a struct in the FuzzyOutputClass.
  status = lpFC->Accumulation();
  if ( status != 0 ) {
    ErrMsg("Accumulation Failed.", "", status);
    return status;
  }
    
  // STEP 5: DEFUZZIFICATION
  // Conversion of linguistic output variables into crisp values
  status = lpFC->Defuzzification();
  if ( status != 0 ) {
    ErrMsg("Defuzzification Failed.", "", status);
    return status;
  }

  // Write output data if requested
  if ( not lpFC->OutputFileName().empty() ) {
    status = lpFC->WriteOutput();
    if ( status != 0 ) {
      ErrMsg("WriteOutput() Failed.", lpFC->OutputFileName(), status);
      return status;
    }
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_SeriesInput
//
// Purpose: Run control on contents of FuzzyControl.InputData map
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_SeriesInput(FuzzyControlClass* lpFC, int numDataPoints) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_SeriesInput()", "Invalid FuzzyControlClass", status);
    return status;
  }

  // Process each input data value, ReadInputDataFile() returns numDataPoints
  for ( int i = 0; i < numDataPoints; i++ ) {
    // for each point in the InputData vector (timeseries)

    // STEP 1: FUZZIFICATION
    status = lpFC->Fuzzification(i);
    if ( status != 0 ) {
      ErrMsg( "Fuzzification Failed.", "", status );
      break;
    }

    // STEP 2: INFERENCE: AGGREGATION
    status = lpFC->Aggregation();
    if ( status != 0 ) {
      ErrMsg("Aggregation Failed.", "", status);
      break;
    }

    // STEP 3: INFERENCE: ACTIVATION
    status = lpFC->Activation();
    if ( status != 0 ) {
      ErrMsg("Activation Failed.", "", status);
      break;
    }

    // STEP 4: INFERENCE: ACCUMULATION
    status = lpFC->Accumulation();
    if ( status != 0 ) {
      ErrMsg("Accumulation Failed.", "", status);
      break;
    }
    
    // STEP 5: DEFUZZIFICATION
    status = lpFC->Defuzzification();
    if ( status != 0 ) {
      ErrMsg("Defuzzification Failed.", "", status);
      break;
    }

    // Write output data if requested
    if ( not lpFC->OutputFileName().empty() ) {
      status = lpFC->WriteTimestepOutput( i );
      if ( status != 0 ) {
	ErrMsg("WriteTimestepOutput() Failed.", lpFC->OutputFileName(), status);
	break;
      }
    }

  } // for ( int i = 0; i < numDataPoints; i++ )
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_Fuzzify
//
// Purpose: FUZZIFICATION
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_Fuzzify( FuzzyControlClass* lpFC, 
			  string varName, double inputValue ) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_Fuzzify()", "Invalid FuzzyControlClass", status);
    return status;
  }

  // Conversion of input value to linguistic variables
  status = lpFC->FuzzifyInput(varName, inputValue);
  if ( status != 0 ) {
    ErrMsg( "Fuzzification Failed on variable", varName, status );
    ErrMsg( "Fuzzification Failed on value", inputValue, status );
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_Aggregation
//
// Purpose: INFERENCE: AGGREGATION
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_Aggregation(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_Aggregation()", "Invalid FuzzyControlClass", status);
    return status;
  }

  // Determine degree of conformance of the rule conditions 
  // from the degree of membership of the condition terms.
  status = lpFC->Aggregation();
  if ( status != 0 ) {
    ErrMsg("Aggregation Failed.", "", status);
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_Activation
//
// Purpose: INFERENCE: ACTIVATION
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_Activation(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_Activation()", "Invalid FuzzyControlClass", status);
    return status;
  }
  // Activation (assign the value) of the IF-THEN conclusion
  status = lpFC->Activation();
  if ( status != 0 ) {
    ErrMsg("Activation Failed.", "", status);
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_Accumulation
//
// Purpose: INFERENCE: ACCUMULATION
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_Accumulation(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_Accumulation()", "Invalid FuzzyControlClass", status);
    return status;
  }
  // Combination of the weighted results of the rules into an overall result
  status = lpFC->Accumulation();
  if ( status != 0 ) {
    ErrMsg("Accumulation Failed.", "", status);
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_Defuzzification
//
// Purpose: DEFUZZIFICATION
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_Defuzzification(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_Defuzzification()", 
	   "Invalid FuzzyControlClass", status);
    return status;
  }
  // Conversion of linguistic output variables into crisp values
  status = lpFC->Defuzzification();
  if ( status != 0 ) {
    ErrMsg("Defuzzification Failed.", "", status);
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControlReadFCL
//
// Purpose: Read & Parse the FCL file, init all FuzzyControl maps
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_ReadFCL(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_ReadFCL()", "Invalid FuzzyControlClass", status);
    return status;
  }
  // Open the FCL file, buffer contents, close file
  status = lpFC->ReadFCLFile();
  if ( status != 0 ) {
    ErrMsg( "Failed to read FCL file", lpFC->FCLFile(), status );
    return status;
  }

  // Parse the FCL file, create FuzzyControl variables
  status = lpFC->ParseFCLFile();
  if ( status != 0 ) {
    ErrMsg( "Failed to parse FCL file", lpFC->FCLFile(), status );
    return status;
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_IO_Files
//
// Purpose: Open I/O files, buffer InputData
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_IO_Files(FuzzyControlClass* lpFC, string* inputDataFileName,
			  string* outputDataFileName, int *numPointsRead) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_IO_Files()", "Invalid FuzzyControlClass", status);
    return status;
  }
  // Read in the fuzzy data input from a file
  // Buffer into the FuzzyControl::InputData map
  int numDataPoints = 0;
  numDataPoints = lpFC->ReadInputDataFile( inputDataFileName );
  if ( numDataPoints < 0 ) {
    status = -1;
    ErrMsg( "Failed to read input data file", 
	    *inputDataFileName, numDataPoints );
    return status;
  }
  *numPointsRead = numDataPoints;

  // Open the output data file if requested
  if ( not outputDataFileName->empty() ) {
    lpFC->OutputFileName() = *outputDataFileName;
    status = lpFC->OpenOutputFile( outputDataFileName );
    if ( status != 0 ) {
      return status;
    }
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_ReadDataFile
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_ReadDataFile(FuzzyControlClass* lpFC, 
			       string *inputDataFileName,
			       int    *numPointsRead) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_ReadDataFile()", "Invalid FuzzyControlClass", status);
    return status;
  }
  // Read in the fuzzy data input from a file
  // Buffer into the FuzzyControl::InputData map
  int numDataPoints = 0;
  numDataPoints = lpFC->ReadInputDataFile( inputDataFileName );
  if ( numDataPoints < 0 ) {
    status = -1;
    ErrMsg( "Failed to read input data file", 
	    *inputDataFileName, numDataPoints );
    return status;
  }
  *numPointsRead = numDataPoints;
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_OpenOutputFile
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_OpenOutputFile(FuzzyControlClass* lpFC, 
				string* outputDataFileName) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_OpenOutputFile()", 
	   "Invalid FuzzyControlClass", status);
    return status;
  }
  // Open the output data file if requested
  if ( not outputDataFileName->empty() ) {
    lpFC->OutputFileName() = *outputDataFileName;
    status = lpFC->OpenOutputFile( outputDataFileName );
    if ( status != 0 ) {
      return status;
    }
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_WriteOutputFile
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_WriteOutputData(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_WriteOutputData()", 
	   "Invalid FuzzyControlClass", status);
    return status;
  }
  status = lpFC->WriteOutput();
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_WriteOutputHeader
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_WriteOutputHeader(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_WriteOutputHeader()", 
	   "Invalid FuzzyControlClass", status);
    return status;
  }
  status = lpFC->WriteOutputHeader();
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_CloseOutputFile
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0 = OK, , nonzero = ERR
//--------------------------------------------------------------
int FuzzyControl_CloseOutputFile(FuzzyControlClass* lpFC) {

  int status = 0;
  if (not lpFC) {
    status = -1;
    ErrMsg("FuzzyControl_CloseOutputFile()", 
	   "Invalid FuzzyControlClass", status);
    return status;
  }
  // Close the output data file if requested
  if ( not lpFC->OutputFileName().empty() ) {
    status = lpFC->CloseOutputFile();
    if ( status != 0 ) {
      ErrMsg("CloseOutputFile() Failed.", lpFC->OutputFileName(), status);
    }
  }
  return status;
}

//--------------------------------------------------------------
// FuzzyControl_PrintKeywordMap
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0
//--------------------------------------------------------------
int FuzzyControl_PrintKeywordMap(FuzzyControlClass* lpFC) {

  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintKeywordMap()", "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo map to console
  PrintKeywordMap(&(lpFC->Keywords()));
  return 0;
}

//--------------------------------------------------------------
// FuzzyControl_PrintInputVariableMap
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0
//--------------------------------------------------------------
int FuzzyControl_PrintInputVariableMap(FuzzyControlClass* lpFC) {

  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintInputVariableMap()", 
	   "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo map to console
  PrintInputVariableMap(&(lpFC->InputVariablesMap()));
  return 0;
}

//--------------------------------------------------------------
// FuzzyControl_PrintOutputVariableMap
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0
//--------------------------------------------------------------
int FuzzyControl_PrintOutputVariableMap(FuzzyControlClass* lpFC) {

  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintOutputVariableMap()", 
	   "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo map to console
  PrintOutputVariableMap(&(lpFC->OutputVariablesMap()));
  return 0;
}

//--------------------------------------------------------------
// FuzzyControl_PrintRuleMap
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0
//--------------------------------------------------------------
int FuzzyControl_PrintRuleMap(FuzzyControlClass* lpFC) {

  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintRuleMap()", "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo map to console
  PrintRuleMap(&(lpFC->RulesMap()));
  return 0;
}

//--------------------------------------------------------------
// FuzzyControl_PrintFuzzifiedInput
//
// Purpose: 
//
// Arguments: pointer to FuzzyControlClass
//           
// Return: status, 0
//--------------------------------------------------------------
int FuzzyControl_PrintFuzzifiedInput(FuzzyControlClass* lpFC) {
  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintFuzzifiedInput()", 
	   "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo input variable fuzzification values to console
  PrintFuzzifiedInput(&(lpFC->InputVariablesMap()));
  return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int FuzzyControl_PrintAggregation(FuzzyControlClass* lpFC) {
  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintAggregation()", "Invalid FuzzyControlClass", -1);
    return -1;
  }
  ErrMsg("FuzzyControl_PrintAggregation", "Not supported", 0);
  return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int FuzzyControl_PrintActivation(FuzzyControlClass* lpFC) {
  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintActivation()", "Invalid FuzzyControlClass", -1);
    return -1;
  }
  ErrMsg("FuzzyControl_PrintActivation", "Not supported", 0);
  return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int FuzzyControl_PrintAccumulation(FuzzyControlClass* lpFC) {
  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintAccumulation()", "Invalid FuzzyControlClass", -1);
    return -1;
  }
  ErrMsg("FuzzyControl_PrintAccumulation", "Not supported", 0);
  return 0;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int FuzzyControl_PrintDefuzzifiedOutput(FuzzyControlClass* lpFC) {
  if (not lpFC) {
    ErrMsg("FuzzyControl_PrintDefuzzifiedOutput()", 
	   "Invalid FuzzyControlClass", -1);
    return -1;
  }
  // Echo output variable defuzzification values to console
  PrintDefuzzifiedOutput(&(lpFC->OutputVariablesMap()));
  return 0;
}
