
///////////////////////////////////////////////////////////////////////////////////
// fclapi.cc
//
// File to demonstrate FuzzyControl API
//
///////////////////////////////////////////////////////////////////////////////////

// Import the FuzzyControl definitions
#include "FuzzyControl.h"

int main(int argc, char* argv[])
{
  int    status = 0;
  string FCLFileName;        // FCL text file
  string inputDataFileName;  // Input timeseries data for FCL file
  string outputDataFileName; // Optional output data file

  ConsoleMsg("->", "fclapi()", status);

  // Get the FCL file name, I/O file names.
  //
  // FCLFileName must point to a valid FCL text file.
  //
  // inputDataFileName must point to a file which defines input
  // variable timeseries values. format of the file is:
  //
  //          varName1,    varName2,    ..., varNameN
  //          var1_value1, var2_value1, ..., varN_value1
  //          var1_value2, var2_value2, ..., varN_value2
  //          ...,         ...,         ..., ...,
  //          var1_valueM, var2_valueM, ..., varN_valueM
  //
  // First line is list of input variable names, the variable
  // names must match those in the FCLFileName file.
  // Subsequent lines are list of input variable values.
  //
  if (argc < 2) {
    ConsoleMsg("No input files specified", "use test.fcl, test.in, test.out", status);
    FCLFileName        = "test.fcl";
    inputDataFileName  = "test.in";
    outputDataFileName = "test.out";
  }
  else {
    if ( argc < 3 ) {
      status = -1;
      ErrMsg("Usage", "fclapi fcl_file input_data_file [output_data_file]", status);
      return status;  
    }
    FCLFileName        = argv[1];
    inputDataFileName  = argv[2];
    if ( argc > 3 )
      outputDataFileName = argv[3];
  }

  //------------------------------------------------------------
  // This section illustrates batch processing of a timeseries
  // input file

  // Instantiate the main FuzzyControlClass, initialize keywords map
  FuzzyControlClass *lpFC = 0;
  FuzzyControlClass FuzzyControl(FCLFileName);
  lpFC = &FuzzyControl;

  // Open the FCL file. Parse the FCL file and create/initialize
  // the FuzzyControl data structures
  status = FuzzyControl_ReadFCL(lpFC);
  if ( status ) return status;

  // Read in the timeseries data file, open an output data file
  int numPointsRead = 0;
  status = FuzzyControl_IO_Files(lpFC, &inputDataFileName,
				 &outputDataFileName, &numPointsRead);
  if ( status ) return status;

  // Run the FuzzyControl on the input data, write output data
  status = FuzzyControl_SeriesInput(lpFC, numPointsRead);
  if ( status ) return status;

  // close the output file
  status = FuzzyControl_CloseOutputFile(lpFC);
  if ( status ) return status;

#ifdef PROCESS_ALL
  //------------------------------------------------------------
  // This section illustrates individual point processing

  // Instantiate the main FuzzyControlClass
  FuzzyControlClass FuzzyControl2(FCLFileName);
  lpFC = &FuzzyControl2;

  // Open/Parse the FCL file.
  status = FuzzyControl_ReadFCL(lpFC);
  if ( status ) return status;

  // Open an output data file if requested
  if ( not outputDataFileName.empty() ) {
    string outputDataFileName2 = "2." + outputDataFileName;
    numPointsRead = 0;
    status = FuzzyControl_OpenOutputFile(lpFC, &outputDataFileName2);
    if ( status ) return status;
    // Write a header of variable names to the output file
    status = FuzzyControl_WriteOutputHeader(lpFC);
    if ( status ) return status;
  }

  // Conversion of input values to linguistic variables
  // Fuzzify an input value for each of the two input variables
  status = FuzzyControl_Fuzzify(lpFC, "temp", 35.);
  if ( status ) return status;

  status = FuzzyControl_Fuzzify(lpFC, "pressure", 250.);
  if ( status ) return status;

  // Run the FuzzyControl on the input data
  status = FuzzyControl_SingleInput(lpFC);
  if ( status ) return status;

  // Echo the fuzzified input values to the console
  FuzzyControl_PrintFuzzifiedInput(lpFC);
  // Echo the defuzzified output values to the console
  FuzzyControl_PrintDefuzzifiedOutput(lpFC);

  // close the output file
  if ( not outputDataFileName.empty() ) {
    status = FuzzyControl_CloseOutputFile(lpFC);
    if ( status ) return status;
  }

  //------------------------------------------------------------
  // This section illustrates individual point processing
  // with individual calls to each fuzzy processing section

  // Instantiate the main FuzzyControlClass
  FuzzyControlClass FuzzyControl3(FCLFileName);
  lpFC = &FuzzyControl3;

  // Open/Parse the FCL file.
  status = FuzzyControl_ReadFCL(lpFC);
  if ( status ) return status;

  // Open an output data file if requested
  if ( not outputDataFileName.empty() ) {
    string outputDataFileName3 = "3." + outputDataFileName;
    numPointsRead = 0;
    status = FuzzyControl_OpenOutputFile(lpFC, &outputDataFileName3);
    if ( status ) return status;
  }

  // STEP 1: FUZZIFICATION
  // Conversion of input values to linguistic variables
  // Fuzzify an input value for each of the two input variables
  status = FuzzyControl_Fuzzify(lpFC, "temp", 35.);
  if ( status ) return status;

  status = FuzzyControl_Fuzzify(lpFC, "pressure", 250.);
  if ( status ) return status;

  // STEP 2: INFERENCE: AGGREGATION
  // Determine degree of conformance of the rule conditions
  // from the degree of membership of the condition terms.
  status = FuzzyControl_Aggregation(lpFC);
  if ( status ) return status;

  // STEP 3: INFERENCE: ACTIVATION
  // Activation (assign the value) of the IF-THEN conclusion
  status = FuzzyControl_Activation(lpFC);
  if ( status ) return status;

  // STEP 4: INFERENCE: ACCUMULATION
  // Combination of the weighted results of the rules into an overall result
  status = FuzzyControl_Accumulation(lpFC);
  if ( status ) return status;

  // STEP 5: DEFUZZIFICATION
  // Conversion of linguistic output variables into crisp values
  status = FuzzyControl_Defuzzification(lpFC);
  if ( status ) return status;

  // Echo the defuzzified output values to the console
  FuzzyControl_PrintDefuzzifiedOutput(lpFC);

  // Write out the defuzzified output values
  status = FuzzyControl_WriteOutputHeader(lpFC);
  if ( status ) return status;
  status = FuzzyControl_WriteOutputData(lpFC);
  if ( status ) return status;

  // Close the output data file
  if ( not outputDataFileName.empty() ) {
    status = FuzzyControl_CloseOutputFile(lpFC);
    if ( status ) return status;
  }
#endif
  ConsoleMsg("<-", "fclapi()", status);
}

///////////////////////////////////////////////////////////////////////////////////
