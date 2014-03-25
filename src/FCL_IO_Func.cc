#include "FuzzyControl.h"

//------------------------------------------------------------
// ReadFCLFile
//
// Purpose: Buffer FCL file into vector of strings
//
// Arguments:
//           
// Return:   
//------------------------------------------------------------
int FuzzyControlClass::ReadFCLFile() {

  // Open the FCL input file for reading
  FCLFileStream.open( FCLFileName.c_str(), ios::in );
  if ( not FCLFileStream ) {
    ErrMsg( "Failed to open file:", FCLFileName, -1 );
    return -1;
  }
  DebugAllMsg( "Opened FCL file ", FCLFileName, 0 );

  const string& whitespace = " \t\r\n";
  const char&   CR         = '\r';

  // Read each line into a string, store in vector
  // JP I would like to get an iterator that would terminate on \n 
  // instead of whitespace as the istream_iterator does, then this 
  // vector creation could be done all at once:
  //    vector<string> FCLFileVector (inputFCLiterator, eos);
  // Since don't know how to do that, append the vector as lines are read in.
  while ( not FCLFileStream.eof() and FCLFileStream.good() ) {

    // getline default delim is LF '\n' \0x0A
    getline( FCLFileStream, inputFCL );

    // Ignore blank lines
    string::size_type nFirst = inputFCL.find_first_not_of( whitespace );
    if ( nFirst == std::string::npos ) {
      continue; // no content
    }
    if ( CommentLine( &inputFCL ) ) {
      continue;
    }
    // Strip out Carriage Return CR '\r' \0x0D
    nFirst = inputFCL.find_first_of( CR );
    if ( nFirst != std::string::npos ) {
       inputFCL.erase( nFirst, 1 );
       nFirst = inputFCL.find_first_of( CR );
       if ( nFirst != std::string::npos ) {
	 DebugMsg( "Found multiple CR in FCL file line", FCLFileName, -1 );
	 return -1;
       }
    }
    
    FCLFileVector.push_back( inputFCL );
  }

  FCLFileStream.close();
  DebugAllMsg( "Closed FCL file ", FCLFileName, status );

  // Dump read in file contents to console
  DebugAllMsg( "START FCL", "", status );
  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();
  while ( FCLFileIterator != FCLFileVector.end() ) {
    DebugAllMsg( "FCL", FCLFileIterator->c_str(), status );
    FCLFileIterator++;
  }
  DebugAllMsg("END FCL", "", status);
  return 0;
}

//------------------------------------------------------------
// ReadInputDataFile
//
// Purpose: Buffer Fuzzy Variable Input data values into a 
//          vector of doubles, one vector for each variable
//
// Format of the file is:
//          varName1,    varName2,    ..., varNameN
//          var1_value1, var2_value1, ..., varN_value1
//          var1_value2, var2_value2, ..., varN_value2
//          ...,         ...,         ..., ...,
//          var1_valueM, var2_valueM, ..., varN_valueM
//
//  First line is list of input variable names
//  Subsequent lines are list of input variable values
// 
//  A varName of inputDataLabel is treated as a string and stored 
//  in the InputLabels vector, all others converted to double and
//  stored in InputData map
//
// Arguments:
//           
// Return:  Number of points read, or error code
//------------------------------------------------------------
int FuzzyControlClass::ReadInputDataFile( string *fileName ) {

  ifstream InputDataStream;
  string   inputLine;
  vector< string > inputWords;
  vector< string > varNames;
  vector< string >::const_iterator iwi;
  map< string, FuzzyInputClass* >::const_iterator fii;

  vector< double > *lpDataVector  = 0;

  int  numPointsRead = 0;
  int  numVariables  = 0;

  const string& whitespace = " \t\r\n";
  const char&   CR         = '\r';

  // Open the FuzzyInput variable data file for reading
  InputDataStream.open( fileName->c_str(), ios::in );
  if ( not InputDataStream ) {
    ErrMsg( "Failed to open input data file:", *fileName, -1 );
    return -1;
  }
  DebugAllMsg( "Opened input data file ", *fileName, 0 );

  // Get input variable names on the first line
  getline( InputDataStream, inputLine );

  // Check for blank line
  string::size_type nFirst = inputLine.find_first_not_of( whitespace );
  if ( nFirst == std::string::npos ) {
    ErrMsg( "Input data file first line is blank, need input variable names",
	    *fileName, -1 );
    return -1;
  }
  // Strip out Carriage Return CR '\r' \0x0D
  nFirst = inputLine.find_first_of( CR );
  if ( nFirst != std::string::npos ) {
    inputLine.erase( nFirst, 1 );
    nFirst = inputLine.find_first_of( CR );
    if ( nFirst != std::string::npos ) {
      DebugMsg( "Found multiple CR in FCL file line", FCLFileName, -1 );
      return -1;
    }
  }
  
  // Split the variable names
  status = SplitLine( &inputWords, &inputLine, &InputDelimeters() );

  if ( status != 0 ) {
    ErrMsg( "Failed to split input variable names from input data file:",
	    *fileName, -2 );
    return -2;
  }

  // Create a map of variable name and column index
  // This is used to select only input variable columns that correspond
  // to VAR_INPUT in the FCL file when reading the data below.
  map< string, vector<string>::size_type > InputVarColMap;

  for ( vector<string>::size_type i = 0; i < inputWords.size(); i++ ) {
    InputVarColMap[ inputWords[ i ] ] = i;
  }

  // Remove inputWords that are not in the FCL file as VAR_INPUT
  // This allows the input data file to have columns that are not
  // explicitly defined as VAR_INPUT in the FCL file.
  vector< string > dataColumnToDelete;
  vector< string >::const_iterator dcdi;

  for ( iwi = inputWords.begin(); iwi != inputWords.end(); ++iwi ) {
    string varName = *iwi;

    if ( InputDataLabel().length() and
         varName.compare( InputDataLabel() ) == 0 ) {
      // varName is a data column label that will be saved in InputLabels
      continue;
    }    

    fii = InputVariables.find( varName );
    if ( fii == InputVariables.end() ) {
      // The varName is not a key in the InputVariable Map
      DebugMsg( "Will delete input variable from inputWords", varName, 0 );
      dataColumnToDelete.push_back( varName );
    }
  }

  vector< string >::iterator fiwi;
  for ( dcdi  = dataColumnToDelete.begin(); 
	dcdi != dataColumnToDelete.end(); ++dcdi ) {
    string varName = *dcdi;
    fiwi = find( inputWords.begin(), inputWords.end(), varName );
    if ( fiwi != inputWords.end() ) {
      inputWords.erase( fiwi );
    }
  }

  numVariables = inputWords.size();

  // Create data vectors for each input variable and store in InputData map.
  for ( iwi = inputWords.begin(); iwi != inputWords.end(); ++iwi ) {
    string varName = *iwi;

    DebugAllMsg( "Data Input Variable", varName, 0 );

    varNames.push_back( varName );
    
    if ( InputDataLabel().length() and
         varName.compare( InputDataLabel() ) == 0 ) {
      // varName is a data column label that will be saved in InputLabels
      continue;
    }
    else {
      // Create a vector<double> to hold the data
      lpDataVector = new vector < double >;
      // stick it in the map
      InputData[ varName ] = lpDataVector;
    }
  }

  // clear the inputWords list since we reuse it for the data
  inputWords.clear();

  // Now read each line and get data values for each input variable
  while ( not InputDataStream.eof() and InputDataStream.good() ) {

    getline( InputDataStream, inputLine );

    nFirst = inputLine.find_first_not_of( whitespace );
    if ( nFirst == std::string::npos ) {
      continue; // no content
    }
    if ( CommentLine( &inputLine ) ) {
      continue;
    }
    // Strip out Carriage Return CR '\r' \0x0D
    nFirst = inputLine.find_first_of( CR );
    if ( nFirst != std::string::npos ) {
       inputLine.erase( nFirst, 1 );
       nFirst = inputLine.find_first_of( CR );
       if ( nFirst != std::string::npos ) {
	 DebugMsg( "Found multiple CR in FCL file line", FCLFileName, -1 );
	 return -1;
       }
    }

    status = SplitLine( &inputWords, &inputLine, &InputDelimeters() );
    if ( status != 0 ) {
      ErrMsg( "Failed to split input variable data from input data file:",
	      *fileName, -3 );
      ErrMsg( "Failed to split input variable data from input:",
	      inputLine, -3 );
      return -3;
    }
    if ( inputWords.begin() == inputWords.end() ) continue;

    // Match the data columns with the InputMap to extract only the
    // VAR_INPUT in the FCL file.
    vector< string >::size_type ivn;

    for ( ivn = 0; ivn < varNames.size(); ivn++ ) {
      string varName = varNames[ ivn ];
      vector<string>::size_type colNum = InputVarColMap[ varName ];

      if ( InputData.count( varName ) > 0 ) {
	// This input data column is in the FCL VAR_INPUT
	DebugAllMsg( "Create Data Input Value", inputWords[ colNum ], 0 );

	// Convert the string to a float and store in InputData
	InputData[ varName ]->push_back( atof( inputWords[ colNum ].c_str() ) );
      }
      else if ( InputDataLabel().length() and varName.compare( InputDataLabel() ) == 0 ) {
	// varName label is not a fuzzy input term, store in InputLabels
	InputLabels.push_back( inputWords[ colNum ] );
      }
    }

    // clear the inputWords list
    inputWords.clear();
    numPointsRead++;
  }

  InputDataStream.close();
  DebugAllMsg("Closed input data file ", *fileName, status);

  return numPointsRead;
}

//------------------------------------------------------------
// OpenOutputFile
//
// Purpose: 
//
// Arguments:
//           
// Return:  
//------------------------------------------------------------
int FuzzyControlClass::OpenOutputFile( string *fileName ) {

  int status = 0;

  OutputDataStream.open(fileName->c_str(), ios::out);
  if ( not OutputDataStream ) {
    status = -1;
    ErrMsg("Failed to open output data file:", *fileName, status);
    return status;
  }
  this->outputFileName = *fileName;
  DebugAllMsg("Opened output data file ", *fileName, status);

  return status;
}

//------------------------------------------------------------
// CloseOutputFile
//
// Purpose: 
//
// Arguments:
//           
// Return:  
//------------------------------------------------------------
int FuzzyControlClass::CloseOutputFile() {

  int status = 0;

  OutputDataStream.close();
  DebugAllMsg("Closed output data file ", this->outputFileName, status);

  return status;
}

//------------------------------------------------------------
// WriteTimestepOutput
//
// Purpose: 
//
// Arguments:
//           
// Return:  
//------------------------------------------------------------
int FuzzyControlClass::WriteTimestepOutput( int index ) {

  int status = 0;
  int i      = 0;

  map <string, FuzzyInputClass*>  :: iterator ivi; // InputVariables
  map <string, FuzzyOutputClass*> :: iterator ovi; // OutputVariables

  FuzzyInputClass*   lpFIC;
  FuzzyOutputClass*  lpFOC;
  double             data;
  string             label;
  vector <double>*   lpDataVector;
  vector <string>*   lpLabelVector;

  vector < string >::size_type inputLabelSize = InputLabels.size();

  // Write a header of inputTerms, OutputTerms
  if ( index == 0 ) {
    OutputDataStream << "index, ";
    if ( inputLabelSize ) {
      OutputDataStream << InputDataLabel() << ", ";
    }
    for ( ivi = InputVariables.begin(); ivi != InputVariables.end(); ++ivi ) {
      lpFIC = ivi->second;
      OutputDataStream << lpFIC->varName << ", ";
    }
    for ( ovi = OutputVariables.begin(); ovi != OutputVariables.end(); ++ovi ) {
      lpFOC = ovi->second;
      OutputDataStream << lpFOC->varName << ", ";
    }
    OutputDataStream << "\n";
  }

  // Write a line of data for this index
  OutputDataStream << index << ", ";
  if ( inputLabelSize ) {
    OutputDataStream << InputLabels[ index ] << ", ";
  }
  // Write the input, output data values
  for ( ivi = InputVariables.begin(); ivi != InputVariables.end(); ++ivi ) {
    lpFIC = ivi->second;
    // process the variable with the ith data point from it's vector
    lpDataVector = InputData[ lpFIC->varName ];
    data = (*lpDataVector)[ index ];
    OutputDataStream << data << ", ";
  }
  for ( ovi = OutputVariables.begin(); ovi != OutputVariables.end(); ++ovi ) {
    lpFOC = ovi->second;
    OutputDataStream << lpFOC->defuzzOut << ", ";
  }
  OutputDataStream << "\n";
    
  return status;
}

//------------------------------------------------------------
// WriteOutput
//
// Purpose: 
//
// Arguments:
// 
// Return:  
//------------------------------------------------------------
int FuzzyControlClass::WriteOutput() {

  int status = 0;

  map <string, FuzzyOutputClass*> :: iterator ovi; // OutputVariables

  FuzzyOutputClass* lpFOC;

  // Write the output data values
  for ( ovi = OutputVariables.begin(); ovi != OutputVariables.end(); ++ovi ) {
    lpFOC = ovi->second;
    OutputDataStream << lpFOC->defuzzOut << ", ";
  }
  OutputDataStream << "\n";
    
  return status;
}

//------------------------------------------------------------
// WriteOutputHeader
//
// Purpose: 
//
// Arguments:
//           
// Return:  
//------------------------------------------------------------
int FuzzyControlClass::WriteOutputHeader() {

  int status = 0;

  map <string, FuzzyOutputClass*> :: iterator ovi; // OutputVariables
  FuzzyOutputClass* lpFOC;

  for ( ovi = OutputVariables.begin(); ovi != OutputVariables.end(); ++ovi ) {
    lpFOC = ovi->second;
    OutputDataStream << lpFOC->varName << ", ";
  }
  OutputDataStream << "\n";

  return status;
}
