// 
// example.cc
// 
// Load and parse a Fuzzy Control Logic (FCL) File
//

#include "FuzzyControl.h"

int main(int argc, char* argv[])
{
  int status = 0;
  string FCLFileName;

  // Get the FCL file name
  if (argc < 2) {
    DebugMsg("No input files specified, use test.fcl", "", status);
    FCLFileName = "test.fcl";
  }
  else {
    FCLFileName = argv[1];
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

  return status;
}

///////////////////////////////////////////////////////////////////////////////////
