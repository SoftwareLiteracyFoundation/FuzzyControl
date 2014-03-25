#include "FuzzyControl.h"

//--------------------------------------------------------------
// FindFCLKeywordFromMap
//
// Purpose: 
//
// Arguments:
//
// Return:   FCL_keyword* pointer to keyword struct
//--------------------------------------------------------------
FCL_keyword* FuzzyControlClass::FindFCLKeywordFromMap ( string key, 
							bool err = true ) {

  if ( keywords.find(key) == keywords.end() ) {
    if ( err ) {
      ErrMsg("Failed to find keyword in map" , key, -1);
    }
    return 0;
  }
  return keywords[key];
}

//--------------------------------------------------------------
// SplitLine
//
// Purpose: like Python string.split()
//
// Arguments: splitString : pointer to vector<string> where the
//                          split words are stacked
//            inString    : pointer to string to be split
//            delimeters  : pointer to string of delimeters
//
// Note:  A typical delimeter string: delimeters = " \t,\n:;()"
//           
// Return: status
//--------------------------------------------------------------
int FuzzyControlClass::SplitLine( vector<string>* splitString, 
				  string* inString, 
				  string* delimeters ) {
  int status = 0;
  string::size_type pos       = 0;
  string::size_type wordStart = 0;
  string::size_type wordEnd   = 0;
  string::size_type eos       = 0;

  bool foundStart = false;
  bool foundEnd   = false;

  string word;
  string localString = *inString;

  eos = localString.length();

  while ( pos <= eos ) {
    if ( not foundStart ) {
      if ( delimeters->find( localString[pos] ) == delimeters->npos ) {
	// this char (localString[pos]) is not a delimeter
	wordStart  = pos;
	foundStart = true;
	pos++;
	continue;
      }
    }
    if ( foundStart and not foundEnd ) {
      if ( delimeters->find( localString[pos] ) != delimeters->npos 
	   or pos == eos ) {
	// this char (localString[pos]) is a delimeter or end of the string
	wordEnd  = pos;
	foundEnd = true;
      }
    }
    if ( foundStart and foundEnd ) {
      foundStart = false;
      foundEnd   = false;
      word = localString.substr( wordStart, wordEnd - wordStart );
      splitString->push_back( word );
      DebugAllMsg( "Found word", word, 0 );
    }
    if ( pos == eos ) {
      break;
    }
    pos++;
  }

  // remove leading/trailing whitespace in each word
  const string& whitespace = " \t\r\n";
  for ( vector<string>::iterator vi = splitString->begin();
	vi != splitString->end(); ++vi ) {
    
    string* str = &(*vi);
    string::size_type nFirst = str->find_first_not_of( whitespace );
    if ( nFirst == std::string::npos ) {
      continue; // no content
    }

    // Is this a memory leak? Need to erase or resize?
    if ( nFirst > 0 ) {
      str->erase( 0, nFirst );
    }

    string::size_type nLast = str->find_last_not_of( whitespace );

    if ( nLast != std::string::npos ) {
      str->erase( nLast, str->length() - nLast - 1 );
    }    
  }

#ifdef DEBUG_ALL
  vector<string>::iterator stri = splitString->begin();
  cout << "\n>======= splitString ===========\n[";
  cout << localString << "]\n[";
  while ( stri != splitString->end() ) {
    cout << *stri << " ";
    stri++;
  } 
  cout << "]\n<======= splitString ===========\n";
#endif
  return status;
}

//--------------------------------------------------------------
// FindNoWhiteSpace
//
// Purpose: iterate until no space or tab is seen
//          Include carriage return '\r' and linefeed '\n'
//
// Arguments:
//
// Return:   position in string
//--------------------------------------------------------------
string::size_type FuzzyControlClass::FindNoWhiteSpace( 
  string str, 
  string::size_type offset, 
  int direction ) {

  string::size_type pos = offset;
  string::size_type eos = str.length();

  if (direction == forward) {
    while ( str[pos] == ' ' || str[pos] == '\t' 
	    || str[pos] == '\r' || str[pos] == '\n') {
      if ( pos >= eos ) return eos;
      pos++;
    }
  }
  else if (direction == back) {
    while ( str[pos] == ' ' || str[pos] == '\t' 
	    || str[pos] == '\r' || str[pos] == '\n') {
      if ( pos <= 0 ) return 0;
      pos--;
    }
  }
  else {
    pos = 0;
    ErrMsg("FindNoWhiteSpace() Invalid direction " , str, -1);
  }

  return pos;
}

//--------------------------------------------------------------
// CommentLine
//
// Purpose: classify FCL line as comment or not
//          Assumes that "//" is the START of the comment line
//          A line that has any non-whitespace before "//" will
//          not be recognized as a comment line. 
//          Note that the FCL comment is apparently (* *)
//
// Arguments: FCLLine, string of current line
//           
// Return:   true, false
//--------------------------------------------------------------
bool FuzzyControlClass::CommentLine ( string *line ) {

  string::size_type commentPosition = 0;
  // Find first non-whitespace in line
  commentPosition = FindNoWhiteSpace( *line, 0, forward );
  if ( line->substr( commentPosition, 2) == "//" ) {
    return true;
  }
  if ( line->substr( commentPosition, 2) == "(*" ) {
    return true;
  }
  return false;
}

//--------------------------------------------------------------
// GetLine
//
// Purpose: Form a complete line by looking for ';'
//
// Arguments: FCLFileIterator, FCLFileLine
//           
// Return:   string line filled with text to ';'
//           int number of iterations to apply to FCLFileIterator
//           (the number of physical lines that were read in.)
//--------------------------------------------------------------
int FuzzyControlClass::GetLine ( vector<string>::iterator FCLFileIterator, 
				 string* line ) {
  int increment = 0;

  string newLine = *FCLFileIterator;
  string::size_type EOLPosition = FCLFileIterator->find(';');

  newLine     = *FCLFileIterator;
  EOLPosition = FCLFileIterator->find(';');

  while ( EOLPosition == FCLFileIterator->npos ) {
    FCLFileIterator++;
    increment++;

    if ( FCLFileIterator == FCLFileVector.end() ) {
      increment = -1;
      return increment;
    }
    EOLPosition = FCLFileIterator->find(';');
    newLine = newLine + " " + *FCLFileIterator;
  }

  *line = newLine;

#ifdef DEBUG_ALL
  cout << "GetLine("<< increment << ") : [" << *line << "]\n";
#endif

  return increment;
}

//--------------------------------------------------------------
// CheckFuzzyTermParen
//
// Purpose: Validate that '()' occur in sequence pairs
//
// Arguments: FCLFileLine
//           
// Return: status
//--------------------------------------------------------------
int FuzzyControlClass::CheckFuzzyTermParen ( string* inString ) {

  int status = 0;
  string::size_type pos = 0;
  string::size_type eos = 0;
  char firstParen  = 0;
  char secondParen = 0;

  bool foundFirst  = false;
  bool foundSecond = false;

  string localString = *inString;

  eos = localString.length();

  // Validate that (..) occur in sequence 
  while ( pos <= eos ) {
    if ( localString[pos] == '(' or localString[pos] == ')' ) {
      if ( not foundFirst ) {
	foundFirst = true;
	firstParen = localString[pos];
      }
      else if ( foundFirst and not foundSecond ) {
	foundSecond = true;
	secondParen = localString[pos];
	if ( firstParen == secondParen ) {
	  return -1;
	}
      }
      if ( foundFirst and foundSecond ) {
	foundFirst  = false;
	foundSecond = false;
      }
    }
    pos++;
  }
  // validate that there are even numbers of ( )
  if ( foundFirst and not foundSecond ) {
    return -2;
  }

  return status;
}

//--------------------------------------------------------------
// LoadFCLKeywords
//
// Purpose: Initialize the keywords map with FCL keywords
//
// Arguments:
//           map<string, FCL_keyword*>& keywords - pointer to map
//           of keywords.
//           
// Return:   int - status 0 if OK
//--------------------------------------------------------------
int FuzzyControlClass::LoadFCLKeywords(map<string, FCL_keyword*>& keywords) {
  int status = 0;
  int i = 0;
  // Initialize the keyword map
  // keys are keyword, values are FCL_keyword objects
  FCL_keyword* ACCU_FCL_keyword = new FCL_keyword;
  if ( not ACCU_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create ACCU keyword", status);
    return status;
  }
  ACCU_FCL_keyword->keyword = "ACCU";
  ACCU_FCL_keyword->meaning = "Accumulation method.";
  ACCU_FCL_keyword->index = i;
  i++;
  keywords[ACCU_FCL_keyword->keyword] = ACCU_FCL_keyword;

  FCL_keyword* ACT_FCL_keyword = new FCL_keyword;
  if ( not ACT_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create ACT keyword", status);
    return status;
  }
  ACT_FCL_keyword->keyword = "ACT";
  ACT_FCL_keyword->meaning = "Actuation method.";
  ACT_FCL_keyword->index = i;
  i++;
  keywords[ACT_FCL_keyword->keyword] = ACT_FCL_keyword;

  FCL_keyword* AND_FCL_keyword = new FCL_keyword;
  if ( not AND_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create AND keyword", status);
    return status;
  }
  AND_FCL_keyword->keyword = "AND";
  AND_FCL_keyword->meaning = "AND operator.";
  AND_FCL_keyword->index = i;
  i++;
  keywords[AND_FCL_keyword->keyword] = AND_FCL_keyword;

  FCL_keyword* ASUM_FCL_keyword = new FCL_keyword;
  if ( not ASUM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create ASUM keyword", status);
    return status;
  }
  ASUM_FCL_keyword->keyword = "ASUM";
  ASUM_FCL_keyword->meaning = "OR operator, alegbraic sum.";
  ASUM_FCL_keyword->index = i;
  i++;
  keywords[ASUM_FCL_keyword->keyword] = ASUM_FCL_keyword;

  FCL_keyword* BDIF_FCL_keyword = new FCL_keyword;
  if ( not BDIF_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create BDIF keyword", status);
    return status;
  }
  BDIF_FCL_keyword->keyword = "BDIF";
  BDIF_FCL_keyword->meaning = "AND operator, bounded difference.";
  BDIF_FCL_keyword->index = i;
  i++;
  keywords[BDIF_FCL_keyword->keyword] = BDIF_FCL_keyword;

  FCL_keyword* BSUM_FCL_keyword = new FCL_keyword;
  if ( not BSUM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create BSUM keyword", status);
    return status;
  }
  BSUM_FCL_keyword->keyword = "BSUM";
  BSUM_FCL_keyword->meaning = "Accumulation method, bounded sum.";
  BSUM_FCL_keyword->index = i;
  i++;
  keywords[BSUM_FCL_keyword->keyword] = BSUM_FCL_keyword;

  FCL_keyword* COA_FCL_keyword = new FCL_keyword;
  if ( not COA_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create COA keyword", status);
    return status;
  }
  COA_FCL_keyword->keyword = "COA";
  COA_FCL_keyword->meaning = "Center of Area defuzzification method.";
  COA_FCL_keyword->index = i;
  i++;
  keywords[COA_FCL_keyword->keyword] = COA_FCL_keyword;

  FCL_keyword* COG_FCL_keyword = new FCL_keyword;
  if ( not COG_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create COG keyword", status);
    return status;
  }
  COG_FCL_keyword->keyword = "COG";
  COG_FCL_keyword->meaning = "Center of Gravity defuzzification method.";
  COG_FCL_keyword->index = i;
  i++;
  keywords[COG_FCL_keyword->keyword] = COG_FCL_keyword;

  FCL_keyword* COGS_FCL_keyword = new FCL_keyword;
  if ( not COGS_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create COGS keyword", status);
    return status;
  }
  COGS_FCL_keyword->keyword = "COGS";
  COGS_FCL_keyword->meaning = "Center of Gravity defuzzification "
                              "of singletons method.";
  COGS_FCL_keyword->index = i;
  i++;
  keywords[COGS_FCL_keyword->keyword] = COGS_FCL_keyword;

  FCL_keyword* DEFAULT_FCL_keyword = new FCL_keyword;
  if ( not DEFAULT_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create DEFAULT keyword", status);
    return status;
  }
  DEFAULT_FCL_keyword->keyword = "DEFAULT";
  DEFAULT_FCL_keyword->meaning = "Default output value in case no "
                                 "rule has fired.";
  DEFAULT_FCL_keyword->index = i;
  i++;
  keywords[DEFAULT_FCL_keyword->keyword] = DEFAULT_FCL_keyword;

  FCL_keyword* DEFUZZIFY_FCL_keyword = new FCL_keyword;
  if ( not DEFUZZIFY_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create DEFUZZIFY keyword", status);
    return status;
  }
  DEFUZZIFY_FCL_keyword->keyword = "DEFUZZIFY";
  DEFUZZIFY_FCL_keyword->meaning = "Defuzzification of output variable.";
  DEFUZZIFY_FCL_keyword->index = i;
  i++;
  keywords[DEFUZZIFY_FCL_keyword->keyword] = DEFUZZIFY_FCL_keyword;

  FCL_keyword* END_DEFUZZIFY_FCL_keyword = new FCL_keyword;
  if ( not END_DEFUZZIFY_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create END_DEFUZZIFY keyword", status);
    return status;
  }
  END_DEFUZZIFY_FCL_keyword->keyword = "END_DEFUZZIFY";
  END_DEFUZZIFY_FCL_keyword->meaning = "End of defuzzification specifications.";
  END_DEFUZZIFY_FCL_keyword->index = i;
  i++;
  keywords[END_DEFUZZIFY_FCL_keyword->keyword] = END_DEFUZZIFY_FCL_keyword;

  FCL_keyword* END_FUNCTION_BLOCK_FCL_keyword = new FCL_keyword;
  if ( not END_FUNCTION_BLOCK_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create END_FUNCTION_BLOCK keyword", status);
    return status;
  }
  END_FUNCTION_BLOCK_FCL_keyword->keyword = "END_FUNCTION_BLOCK";
  END_FUNCTION_BLOCK_FCL_keyword->meaning = "End of function block "
                                            "specifications.";
  END_FUNCTION_BLOCK_FCL_keyword->index = i;
  i++;
  keywords[END_FUNCTION_BLOCK_FCL_keyword->keyword] = 
    END_FUNCTION_BLOCK_FCL_keyword;

  FCL_keyword* END_FUZZIFY_FCL_keyword = new FCL_keyword;
  if ( not END_FUZZIFY_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create END_FUZZIFY keyword", status);
    return status;
  }
  END_FUZZIFY_FCL_keyword->keyword = "END_FUZZIFY";
  END_FUZZIFY_FCL_keyword->meaning = "End of fuzzification specifications.";
  END_FUZZIFY_FCL_keyword->index = i;
  i++;
  keywords[END_FUZZIFY_FCL_keyword->keyword] = END_FUZZIFY_FCL_keyword;

  FCL_keyword* END_OPTIONS_FCL_keyword = new FCL_keyword;
  if ( not END_OPTIONS_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
"Failed to create END_OPTIONS keyword", status);
    return status;
  }
  END_OPTIONS_FCL_keyword->keyword = "END_OPTIONS";
  END_OPTIONS_FCL_keyword->meaning = "End of options specifications.";
  END_OPTIONS_FCL_keyword->index = i;
  i++;
  keywords[END_OPTIONS_FCL_keyword->keyword] = END_OPTIONS_FCL_keyword;

  FCL_keyword* END_RULEBLOCK_FCL_keyword = new FCL_keyword;
  if ( not END_RULEBLOCK_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create END_RULEBLOCK keyword", status);
    return status;
  }
  END_RULEBLOCK_FCL_keyword->keyword = "END_RULEBLOCK";
  END_RULEBLOCK_FCL_keyword->meaning = "End of ruleblock specifications.";
  END_RULEBLOCK_FCL_keyword->index = i;
  i++;
  keywords[END_RULEBLOCK_FCL_keyword->keyword] = END_RULEBLOCK_FCL_keyword;

  FCL_keyword* END_VAR_FCL_keyword = new FCL_keyword;
  if ( not END_VAR_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create END_VAR keyword", status);
    return status;
  }
  END_VAR_FCL_keyword->keyword = "END_VAR";
  END_VAR_FCL_keyword->meaning = "End of input/output variable definitions.";
  END_VAR_FCL_keyword->index = i;
  i++;
  keywords[END_VAR_FCL_keyword->keyword] = END_VAR_FCL_keyword;

  FCL_keyword* FUNCTION_BLOCK_FCL_keyword = new FCL_keyword;
  if ( not FUNCTION_BLOCK_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create FUNCTION_BLOCK keyword", status);
    return status;
  }
  FUNCTION_BLOCK_FCL_keyword->keyword = "FUNCTION_BLOCK";
  FUNCTION_BLOCK_FCL_keyword->meaning = "Start of function block "
                                        "specifications.";
  FUNCTION_BLOCK_FCL_keyword->index = i;
  i++;
  keywords[FUNCTION_BLOCK_FCL_keyword->keyword] = FUNCTION_BLOCK_FCL_keyword;

  FCL_keyword* FUZZIFY_FCL_keyword = new FCL_keyword;
  if ( not FUZZIFY_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create FUZZIFY keyword", status);
    return status;
  }
  FUZZIFY_FCL_keyword->keyword = "FUZZIFY";
  FUZZIFY_FCL_keyword->meaning = "Fuzzification of input variable.";
  FUZZIFY_FCL_keyword->index = i;
  i++;
  keywords[FUZZIFY_FCL_keyword->keyword] = FUZZIFY_FCL_keyword;

  FCL_keyword* IF_FCL_keyword = new FCL_keyword;
  if ( not IF_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create IF keyword", status);
    return status;
  }
  IF_FCL_keyword->keyword = "IF";
  IF_FCL_keyword->meaning = "Start of rule which is followed by the condition.";
  IF_FCL_keyword->index = i;
  i++;
  keywords[IF_FCL_keyword->keyword] = IF_FCL_keyword;

  FCL_keyword* IS_FCL_keyword = new FCL_keyword;
  if ( not IS_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create IS keyword", status);
    return status;
  }
  IS_FCL_keyword->keyword = "IS";
  IS_FCL_keyword->meaning = "Follows linguistic variable in "
                            "condition and conclusion.";
  IS_FCL_keyword->index = i;
  i++;
  keywords[IS_FCL_keyword->keyword] = IS_FCL_keyword;

  FCL_keyword* LM_FCL_keyword = new FCL_keyword;
  if ( not LM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create LM keyword", status);
    return status;
  }
  LM_FCL_keyword->keyword = "LM";
  LM_FCL_keyword->meaning = "Left Most Maximum defuzzification method.";
  LM_FCL_keyword->index = i;
  i++;
  keywords[LM_FCL_keyword->keyword] = LM_FCL_keyword;

  FCL_keyword* MAX_FCL_keyword = new FCL_keyword;
  if ( not MAX_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create MAX keyword", status);
    return status;
  }
  MAX_FCL_keyword->keyword = "MAX";
  MAX_FCL_keyword->meaning = "Maximum accumulation method.";
  MAX_FCL_keyword->index = i;
  i++;
  keywords[MAX_FCL_keyword->keyword] = MAX_FCL_keyword;

  FCL_keyword* METHOD_FCL_keyword = new FCL_keyword;
  if ( not METHOD_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create METHOD keyword", status);
    return status;
  }
  METHOD_FCL_keyword->keyword = "METHOD";
  METHOD_FCL_keyword->meaning = "Method of defuzzification.";
  METHOD_FCL_keyword->index = i;
  i++;
  keywords[METHOD_FCL_keyword->keyword] = METHOD_FCL_keyword;

  FCL_keyword* MIN_FCL_keyword = new FCL_keyword;
  if ( not MIN_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create MIN keyword", status);
    return status;
  }
  MIN_FCL_keyword->keyword = "MIN";
  MIN_FCL_keyword->meaning = "Minimum as AND operator.";
  MIN_FCL_keyword->index = i;
  i++;
  keywords[MIN_FCL_keyword->keyword] = MIN_FCL_keyword;

  FCL_keyword* NC_FCL_keyword = new FCL_keyword;
  if ( not NC_FCL_keyword  ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create NC keyword", status);
    return status;
  }
  NC_FCL_keyword->keyword = "NC";
  NC_FCL_keyword->meaning = "No change of output variable in "
                            "case no rule has fired.";
  NC_FCL_keyword->index = i;
  i++;
  keywords[NC_FCL_keyword->keyword] = NC_FCL_keyword;

  FCL_keyword* NOT_FCL_keyword = new FCL_keyword;
  if ( not NOT_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create NOT keyword", status);
    return status;
  }
  NOT_FCL_keyword->keyword = "NOT";
  NOT_FCL_keyword->meaning = "NOT operator.";
  NOT_FCL_keyword->index = i;
  i++;
  keywords[NOT_FCL_keyword->keyword] = NOT_FCL_keyword;

  FCL_keyword* NSUM_FCL_keyword = new FCL_keyword;
  if ( not NSUM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create NSUM keyword", status);
    return status;
  }
  NSUM_FCL_keyword->keyword = "NSUM";
  NSUM_FCL_keyword->meaning = "Normalized sum accumulation method.";
  NSUM_FCL_keyword->index = i;
  i++;
  keywords[NSUM_FCL_keyword->keyword] = NSUM_FCL_keyword;

  FCL_keyword* OPTIONS_FCL_keyword = new FCL_keyword;
  if ( not OPTIONS_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create OPTIONS keyword", status);
    return status;
  }
  OPTIONS_FCL_keyword->keyword = "OPTIONS";
  OPTIONS_FCL_keyword->meaning = "Definition of optional parameters.";
  OPTIONS_FCL_keyword->index = i;
  i++;
  keywords[OPTIONS_FCL_keyword->keyword] = OPTIONS_FCL_keyword;

  FCL_keyword* OR_FCL_keyword = new FCL_keyword;
  if ( not OR_FCL_keyword  ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create OR keyword", status);
    return status;
  }
  OR_FCL_keyword->keyword = "OR";
  OR_FCL_keyword->meaning = "OR operator.";
  OR_FCL_keyword->index = i;
  i++;
  keywords[OR_FCL_keyword->keyword] = OR_FCL_keyword;

  FCL_keyword* PROD_FCL_keyword = new FCL_keyword;
  if ( not PROD_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create PROD keyword", status);
    return status;
  }
  PROD_FCL_keyword->keyword = "PROD";
  PROD_FCL_keyword->meaning = "Product as AND operator.";
  PROD_FCL_keyword->index = i;
  i++;
  keywords[PROD_FCL_keyword->keyword] = PROD_FCL_keyword;

  FCL_keyword* RANGE_FCL_keyword = new FCL_keyword;
  if ( not RANGE_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create RANGE keyword", status);
    return status;
  }
  RANGE_FCL_keyword->keyword = "RANGE";
  RANGE_FCL_keyword->meaning = "Range of variable for scaling "
                               "of membership function.";
  RANGE_FCL_keyword->index = i;
  i++;
  keywords[RANGE_FCL_keyword->keyword] = RANGE_FCL_keyword;

  FCL_keyword* RM_FCL_keyword = new FCL_keyword;
  if ( not RM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create RM keyword", status);
    return status;
  }
  RM_FCL_keyword->keyword = "RM";
  RM_FCL_keyword->meaning = "Right Most Maximum defuzzification method.";
  RM_FCL_keyword->index = i;
  i++;
  keywords[RM_FCL_keyword->keyword] = RM_FCL_keyword;

  FCL_keyword* RULE_FCL_keyword = new FCL_keyword;
  if ( not RULE_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create RULE keyword", status);
    return status;
  }
  RULE_FCL_keyword->keyword = "RULE";
  RULE_FCL_keyword->meaning = "Start of specification of fuzzy rule.";
  RULE_FCL_keyword->index = i;
  i++;
  keywords[RULE_FCL_keyword->keyword] = RULE_FCL_keyword;

  FCL_keyword* RULEBLOCK_FCL_keyword = new FCL_keyword;
  if ( not RULEBLOCK_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create RULEBLOCK keyword", status);
    return status;
  }
  RULEBLOCK_FCL_keyword->keyword = "RULEBLOCK";
  RULEBLOCK_FCL_keyword->meaning = "Start of specification of rule block.";
  RULEBLOCK_FCL_keyword->index = i;
  i++;
  keywords[RULEBLOCK_FCL_keyword->keyword] = RULEBLOCK_FCL_keyword;

  FCL_keyword* TERM_FCL_keyword = new FCL_keyword;
  if ( not TERM_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create TERM keyword", status);
    return status;
  }
  TERM_FCL_keyword->keyword = "TERM";
  TERM_FCL_keyword->meaning = "Definition of a linguistic term (membership "
                              "function) for a linguistic variable.";
  TERM_FCL_keyword->index = i;
  i++;
  keywords[TERM_FCL_keyword->keyword] = TERM_FCL_keyword;

  FCL_keyword* THEN_FCL_keyword = new FCL_keyword;
  if ( not THEN_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create THEN keyword", status);
    return status;
  }
  THEN_FCL_keyword->keyword = "THEN";
  THEN_FCL_keyword->meaning = "Separates condition from conclusion.";
  THEN_FCL_keyword->index = i;
  i++;
  keywords[THEN_FCL_keyword->keyword] = THEN_FCL_keyword;

  FCL_keyword* VAR_FCL_keyword = new FCL_keyword;
  if ( not VAR_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create VAR keyword", status);
    return status;
  }
  VAR_FCL_keyword->keyword = "VAR";
  VAR_FCL_keyword->meaning = "Definition of local variable(s).";
  VAR_FCL_keyword->index = i;
  i++;
  keywords[VAR_FCL_keyword->keyword] = VAR_FCL_keyword;

  FCL_keyword* VAR_INPUT_FCL_keyword = new FCL_keyword;
  if ( not VAR_INPUT_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create VAR_INPUT keyword", status);
    return status;
  }
  VAR_INPUT_FCL_keyword->keyword = "VAR_INPUT";
  VAR_INPUT_FCL_keyword->meaning = "Definition of input variable(s).";
  VAR_INPUT_FCL_keyword->index = i;
  i++;
  keywords[VAR_INPUT_FCL_keyword->keyword] = VAR_INPUT_FCL_keyword;

  FCL_keyword* VAR_OUTPUT_FCL_keyword = new FCL_keyword;
  if ( not VAR_OUTPUT_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create VAR_OUTPUT keyword", status);
    return status;
  }
  VAR_OUTPUT_FCL_keyword->keyword = "VAR_OUTPUT";
  VAR_OUTPUT_FCL_keyword->meaning = "Definition of output variable(s).";
  VAR_OUTPUT_FCL_keyword->index = i;
  i++;
  keywords[VAR_OUTPUT_FCL_keyword->keyword] = VAR_OUTPUT_FCL_keyword;

  FCL_keyword* WITH_FCL_keyword = new FCL_keyword;
  if ( not WITH_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create WITH keyword", status);
    return status;
  }
  WITH_FCL_keyword->keyword = "WITH";
  WITH_FCL_keyword->meaning = "Definition of weighting factor.";
  WITH_FCL_keyword->index = i;
  i++;
  keywords[WITH_FCL_keyword->keyword] = WITH_FCL_keyword;

  FCL_keyword* INVALID_FCL_keyword = new FCL_keyword;
  if ( not INVALID_FCL_keyword ) { 
    status = -1;
    ErrMsg("FuzzyControlClass::LoadFCLKeywords()", 
	   "Failed to create INVALID keyword", status);
    return status;
  }
  INVALID_FCL_keyword->keyword = "INVALID";
  INVALID_FCL_keyword->meaning = "Invalid keyword.";
  INVALID_FCL_keyword->index = i;
  i++;
  keywords[INVALID_FCL_keyword->keyword] = INVALID_FCL_keyword;

  return status;
}
