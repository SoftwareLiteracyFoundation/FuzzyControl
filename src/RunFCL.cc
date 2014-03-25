
//-----------------------------------------------------------
// RunFCL.cc
//
// Use the FuzzyControl API to process an input file and
// an FCL file. 
//
//-----------------------------------------------------------

// Import the FuzzyControl definitions
#include "FuzzyControl.h"

int main( int argc, char* argv[] )
{
  int    status = 0;
  string FCLFileName;        // FCL text file
  string inputDataFileName;  // Input timeseries data for FCL file
  string outputDataFileName; // Optional output data file
  string inputFileDelimeters;// Delimeters for data input file
  string inputDataLabel;     // Data input column name for row labels

#ifdef DEBUG
  ConsoleMsg("->", "RunFCL()", status);
#endif

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
  if ( argc < 2 ) {
    ConsoleMsg("No input files specified", 
	       "Using test.fcl, test.in, test.out", status);
    FCLFileName        = "test.fcl";
    inputDataFileName  = "test.in";
    outputDataFileName = "test.out";
  }
  else {
    if ( argc < 4 ) {
      status = -1;
      ErrMsg("Usage:", "RunFCL fcl_file input_data_file "
	     "output_data_file [input_label input_file_delimeters]", status);
      return status;  
    }

    FCLFileName        = argv[1];
    inputDataFileName  = argv[2];
    outputDataFileName = argv[3];

    // Handle optional parameters
    if ( argc > 4 ) {
      inputDataLabel = argv[4];
    }

    if ( argc > 5 ) {
      inputFileDelimeters = argv[5];
    }
    else {
      inputFileDelimeters = ",";  // = " ,\t;:";
    }
  }

  //----------------------------------------------------------------
  // Instantiate the main FuzzyControlClass, initialize keywords map
  FuzzyControlClass *lpFC = 0;
  FuzzyControlClass FuzzyControl( FCLFileName, 
				  inputFileDelimeters,
				  inputDataLabel );
  lpFC = &FuzzyControl;

  // Open the FCL file. Parse the FCL file and create/initialize
  // the FuzzyControl data structures
  status = FuzzyControl_ReadFCL( lpFC );
  if ( status ) return status;

  // Read in the timeseries data file, open an output data file
  int numPointsRead = 0;
  status = FuzzyControl_IO_Files( lpFC, &inputDataFileName,
				  &outputDataFileName, &numPointsRead );
  if ( status ) return status;

  // Run the FuzzyControl on the input data, write output data
  status = FuzzyControl_SeriesInput( lpFC, numPointsRead );
  if ( status ) return status;

  // Close the output file
  status = FuzzyControl_CloseOutputFile( lpFC );
  if ( status ) return status;

  string Msg = "Processed " + FCLFileName + " with input " + inputDataFileName + " status";
  ConsoleMsg( "RunFCL", Msg, status );

#ifdef DEBUG
  ConsoleMsg("<-", "RunFCL()", status);
#endif
}
