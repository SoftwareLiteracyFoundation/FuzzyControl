#include "FuzzyControl.h"

//--------------------------------------------------------------
// ParseFCLFile
//
// Purpose: Parse the FCL File, create the I/O data objects, 
//          populate the Input & OutputVariable maps.
//          The arguments to the sub-parsing routines are the 
//          FCL keywords. They are passed in so that if the keywords
//          change, code changes to the sub-parsing routines are 
//          not required, just changes to the LoadFCLKeywords() and
//          the arguments.
//
// Arguments: None
// 
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCLFile() {

  int status = 0;

  // Check FCL file format for basic compatibility
  status = ParseFCL_CheckFile();
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Invalid FCL file format", "", status );
    return status;
  }
  // Parse the input variables
  status = ParseFCL_IO_Vars(keyword_VarIn, keyword_EndVar);
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Failed to parse input variables", "", status );
    return status;
  }
  // Parse the output variables
  status = ParseFCL_IO_Vars(keyword_VarOut, keyword_EndVar);
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Failed to parse output variables", "", status );
    return status;
  }
  // Parse the input variables fuzzify block
  status = ParseFCL_Input_Fuzzify();
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Failed to parse input fuzzify block", "", status );
    return status;
  }
  // echo the InputVariables map to the console if DEBUG
  #ifdef DEBUG
  PrintInputVariableMap(&InputVariables);
  #endif

  // Parse the output variables defuzzify block
  status = ParseFCL_Output_Defuzzify();
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Failed to parse output defuzzify block", 
	    "", status );
    return status;
  }
  // echo the OutputVariables map to the console if DEBUG
  #ifdef DEBUG
  PrintOutputVariableMap(&OutputVariables);
  #endif

  // Parse the rule block
  status = ParseFCL_Rules();
  if ( status != 0 ) {
    ErrMsg( "ParseFCLFile() Failed to parse rule block", "", status );
    return status;
  }  
  // echo the Rules map to the console if DEBUG
  #ifdef DEBUG
  PrintRuleMap(&Rules);
  #endif

  return status;
}

//--------------------------------------------------------------
// ParseFCL_Rules
//
// Purpose: Parse the Rules Block
//
// Arguments:
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCL_Rules() {

  status = 0;
  DebugAllMsg("->ParseFCL_Rules()","", status);

  string FCLFileLine;
  string::size_type stringKeyStart;
  string::size_type stringKeyEnd;
  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileLine = *FCLFileIterator;

    // Look for RULEBLOCK
    stringKeyStart = FCLFileLine.find(keyword_RuleBlock->keyword);
    if ( stringKeyStart != FCLFileLine.npos ) {
      // found RULEBLOCK

      FCL_keyword* AND_Method = 0; // AND method: MIN, PROD, BDIF
      FCL_keyword* OR_Method  = 0; // OR method:  MAX, ASUM, BSUM
      FCL_keyword* ACT_Method = 0; // ACT method: PROD, MIN

      string ruleBlockName;
      string::size_type startRuleBlockNamePosition;
      string::size_type endRuleBlockNamePosition;

      // Find start of ruleblock name
      startRuleBlockNamePosition = 
	FindNoWhiteSpace( FCLFileLine,
			  stringKeyStart + 
			  keyword_RuleBlock->keyword.length(),
			  forward );
      // Find end of ruleblock name
      endRuleBlockNamePosition = FCLFileLine.length();
      endRuleBlockNamePosition = FindNoWhiteSpace( FCLFileLine, 
						   endRuleBlockNamePosition, 
						   back );
      if ( endRuleBlockNamePosition <= startRuleBlockNamePosition ) {
	status = -1;
	ErrMsg("Failed to find end of ruleblock name", 
	       " in [" + FCLFileLine + "]", status);
	DebugAllMsg("<-ParseFCL_Rules()","", status);
	return status;
      }
      // Assign the ruleBlock name string
      ruleBlockName = FCLFileLine.substr(startRuleBlockNamePosition, 
					 endRuleBlockNamePosition - 
					 startRuleBlockNamePosition + 1);
  
      DebugAllMsg("Found RULEBLOCK", ruleBlockName, status);

      // iterate until END_RULEBLOCK
      while (FCLFileIterator != FCLFileVector.end()) {
	string::size_type AndStartPosition;
	string::size_type ActStartPosition;
	string::size_type OrStartPosition;
	string::size_type RuleStartPosition;

	FCLFileIterator++;
	FCLFileLine = *FCLFileIterator;

	// Look for END_RULEBLOCK 
	stringKeyEnd = FCLFileLine.find(keyword_EndRule->keyword);
	if ( stringKeyEnd != FCLFileLine.npos ) {
	  // found END_RULEBLOCK, break to continue looking for next RULEBLOCK
	  break;
	}

	// Handle case where this RULE spans more than one physical line
	// GetLine() will fill in FCLFileLine with the continuous RULE
	int FCLIncrement = GetLine(FCLFileIterator, &FCLFileLine);
	if ( FCLIncrement != -1 and FCLIncrement != 0) {
	  FCLFileIterator += FCLIncrement;
	}
	else if (FCLIncrement == -1) {
	  status = -1;
	  ErrMsg("Encountered end of file reading rule", ruleBlockName, status);
	  DebugAllMsg("<-ParseFCL_Rules()","", status);
	  return status;
	}

	// Look for AND, OR, ACT, RULE
	AndStartPosition  = FCLFileLine.find(keyword_And->keyword);
	ActStartPosition  = FCLFileLine.find(keyword_ACT->keyword);
	OrStartPosition   = FCLFileLine.find(keyword_Or->keyword);
	RuleStartPosition = FCLFileLine.find(keyword_Rule->keyword);

	// Look for AND method, make sure it's not inside a RULE
	if ( AndStartPosition != FCLFileLine.npos and 
	     RuleStartPosition == FCLFileLine.npos ) {

	  // Found AND, get method
	  string AndMethodString;
	  string::size_type AndStartPosition;
	  string::size_type AndEndPosition;
	  
	  AndStartPosition = FCLFileLine.find(":");
	  if ( AndStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule AND delimeter ':'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  AndStartPosition = FindNoWhiteSpace( FCLFileLine, 
					       ++AndStartPosition, forward );
	  AndEndPosition = FCLFileLine.find(";");
	  if ( AndStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule AND end of line delimeter ';'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  AndEndPosition = FindNoWhiteSpace( FCLFileLine, 
					     --AndEndPosition, back );
	  if ( AndEndPosition < AndStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find AND method",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // Get the AND method name
	  AndMethodString = FCLFileLine.substr(AndStartPosition, 
					       AndEndPosition - 
					       AndStartPosition + 1);

	  // Make sure the method is valid: MIN, PROD, BDIF
	  if ( AndMethodString != keyword_Min->keyword and 
	       AndMethodString != keyword_Prod->keyword and
	       AndMethodString != keyword_Bdif->keyword ) {
	    status = -1;
	    ErrMsg("Invalid AND method in ruleblock",
		   AndMethodString + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;	    
	  }
	  // Find the AND Method keyword pointer from keywords map
	  AND_Method = FindFCLKeywordFromMap(AndMethodString, true);
	  if ( not AND_Method ) {
	    status = -1;
	    ErrMsg("Failed to find keyword for AND method",
		   AndMethodString + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  DebugAllMsg("Found AND method", AND_Method->keyword, status);
	}

	// Look for OR method, make sure it's not inside a RULE
	else if ( OrStartPosition != FCLFileLine.npos and 
		  RuleStartPosition == FCLFileLine.npos ) {
	  // Found OR, get method
	  string OrMethodString;
	  string::size_type OrStartPosition;
	  string::size_type OrEndPosition;
	  
	  OrStartPosition = FCLFileLine.find(":");
	  if ( OrStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule OR delimeter ':'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  OrStartPosition = FindNoWhiteSpace( FCLFileLine, 
					       ++OrStartPosition, forward );
	  OrEndPosition = FCLFileLine.find(";");
	  if ( OrStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule OR end of line delimeter ';'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  OrEndPosition = FindNoWhiteSpace( FCLFileLine, 
					     --OrEndPosition, back );
	  if ( OrEndPosition < OrStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find OR method",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // Get the OR Method 
	  OrMethodString = FCLFileLine.substr(OrStartPosition, 
					      OrEndPosition - 
					      OrStartPosition + 1);

	  // Make sure the method is valid: MAX, ASUM, BSUM
	  if ( OrMethodString != keyword_Max->keyword and 
	       OrMethodString != keyword_Asum->keyword and
	       OrMethodString != keyword_Bsum->keyword ) {
	    status = -1;
	    ErrMsg("Invalid OR method in ruleblock" , OrMethodString, status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;	    
	  }
	  // Find the OR Method keyword pointer from keywords map
	  OR_Method = FindFCLKeywordFromMap(OrMethodString, true);
	  if ( not OR_Method ) {
	    status = -1;
	    ErrMsg("Failed to find keyword for OR method",
		   OrMethodString + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  DebugAllMsg("Found OR method", OR_Method->keyword, status);
	}

	// Look for ACT method
	else if ( ActStartPosition != FCLFileLine.npos ) {
	  // Found ACT, get method
	  string ActMethodString;
	  string::size_type ActStartPosition;
	  string::size_type ActEndPosition;
	  
	  ActStartPosition = FCLFileLine.find(":");
	  if ( ActStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule ACT delimeter ':'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  ActStartPosition = FindNoWhiteSpace( FCLFileLine, 
					       ++ActStartPosition, forward );
	  ActEndPosition = FCLFileLine.find(";");
	  if ( ActStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find rule ACT end of line delimeter ';'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  ActEndPosition = FindNoWhiteSpace( FCLFileLine, 
					     --ActEndPosition, back );
	  if ( ActEndPosition < ActStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find ACT method",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // Get the ACT Method 
	  ActMethodString = FCLFileLine.substr(ActStartPosition, 
					       ActEndPosition - 
					       ActStartPosition + 1);

	  // Make sure the method is valid: MIN, PROD
	  if ( ActMethodString != keyword_Min->keyword and 
	       ActMethodString != keyword_Prod->keyword ) {
	    status = -1;
	    ErrMsg("Invalid ACT method in ruleblock",
		   ActMethodString + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;	    
	  }
	  // Find the ACT Method keyword pointer from keywords map
	  ACT_Method = FindFCLKeywordFromMap(ActMethodString, true);
	  if ( not ACT_Method ) {
	    status = -1;
	    ErrMsg("Failed to find keyword in map for ACT method",
		   ActMethodString + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  DebugAllMsg("Found ACT method", ACT_Method->keyword, status);
	}

	// Look for RULE
	else if ( RuleStartPosition != FCLFileLine.npos ) {
	  // Found RULE
	  string ruleName;
	  string::size_type RuleNameStartPosition = 0;
	  string::size_type RuleNameEndPosition   = 0;
	  string::size_type WithStartPosition     = 0;
	  string::size_type IfStartPosition       = 0;
	  string::size_type ThenStartPosition     = 0;
	  string::size_type IsStartPosition       = 0;

	  int nSubConditions  = 0;
	  int nSubConclusions = 0;

	  // Get the rule name
	  RuleNameStartPosition = 
	    FindNoWhiteSpace( FCLFileLine,
			      RuleStartPosition + 
			      keyword_Rule->keyword.length(),
			      forward );
	  // Find end of rule name
	  RuleNameEndPosition = FCLFileLine.find(":");
	  if ( RuleNameEndPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find end of rule name delimeter ':'",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  RuleNameEndPosition = FindNoWhiteSpace( FCLFileLine, 
						  --RuleNameEndPosition, back );
	  if ( RuleNameEndPosition < RuleNameStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find end of ruleblock name",
		   ruleBlockName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // Assign the ruleBlock name string
	  ruleName = FCLFileLine.substr(RuleNameStartPosition, 
					RuleNameEndPosition - 
					RuleNameStartPosition + 1);
  
	  DebugAllMsg("Found RULE", ruleName, status);

	  // Create a FuzzyRuleClass, pass in pointers to needed keywords
	  // I don't like this passing in of keywords, there should be a better
	  // way to get access to the keyword pointers in FuzzyControlClass from
	  // the FuzzyRuleClass
	  FuzzyRuleClass* frc = new FuzzyRuleClass(keyword_Min,
						   keyword_Max,  keyword_Prod,
						   keyword_Bdif, keyword_Asum, 
						   keyword_Bsum);
	  if ( not frc ) {
	    status = -1;
	    ErrMsg("ParseFCL_IO_Rules() Failed to create FuzzyRuleClass for ", 
		   ruleName, status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // set the rule name
	  frc->ruleName = ruleName;
	  // set the methods
	  if ( AND_Method ) {
	    frc->andMethod = AND_Method;
	  }
	  if ( OR_Method ) {
	    frc->orMethod = OR_Method;
	  }
	  if ( ACT_Method ) {
	    frc->actMethod = ACT_Method;
	  }

	  // Validate the rule syntax
	  IfStartPosition   = FCLFileLine.find(keyword_IF->keyword);
	  ThenStartPosition = FCLFileLine.find(keyword_Then->keyword);
	  if ( IfStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find IF in rule",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  if ( ThenStartPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find THEN in rule",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }

          // Read Conditions
	  // Isolate the conditions into a local string 
	  string conditionString;
	  IfStartPosition = FindNoWhiteSpace( FCLFileLine, 
					      IfStartPosition + 
					      keyword_IF->keyword.length(),
					      forward );
	  ThenStartPosition = FindNoWhiteSpace( FCLFileLine, 
						--ThenStartPosition, back );
	  if ( ThenStartPosition <= IfStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find condition in rule",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // conditions are between IF ... THEN
	  conditionString = FCLFileLine.substr( IfStartPosition, 
						ThenStartPosition - 
						IfStartPosition + 1);
	  DebugAllMsg("Condition", conditionString, 0);
	  
	  // count the IS keywords to find the nSubConditions
	  while (( IsStartPosition = 
		   conditionString.find(keyword_IS->keyword, 
					IsStartPosition + 1) )
		    != conditionString.npos ) {
	    nSubConditions++;
	  }
	  frc->nSubConditions = nSubConditions;
	  DebugAllMsg("Found subconditions", nSubConditions, 0);

	  // Find the inputVariables and the inputFuzzifyTerms
	  // First, split the condition string into words
	  vector<string> subCondWords;
	  string delimeters = " \t,\r\n:;()";
	  status = SplitLine( &subCondWords, &conditionString, &delimeters );
	  if ( status != 0 ) {
	    ErrMsg("Failed to split rule conditions",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }

	  // Create a Condition struct, this holds subConditions in 
	  // one of two vectors:
	  // AND_SubConditions or OR_SubConditions 
	  // depending on how the subConditions are
	  // to be combined based on the AND/OR usage
	  Condition* condition = new Condition;
	  if ( not condition ) {
	    status = -1; delete frc;
	    ErrMsg("ParseFCL_IO_Rules() Failed to create condition struct for", 
		   ruleName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  condition->result = 0.;

	  // Iterate through the subCondWords, creating the 
	  // SubCondition structs, and populating the Condition struct 
	  vector<string>::iterator sci = subCondWords.begin();
	  FCL_keyword*     Combination_FCL_KeyWord = 0;
	  FCL_keyword*     FCL_KeyWord   = 0;
	  FuzzyInputClass* inputVariable = 0;
	  FuzzyInputTerm*  inputTerm     = 0;
	  string variableName;
	  string termName;
	  bool   termNegate      = false;
	  bool   conditionNegate = false;

	  while ( sci != subCondWords.end() ) {
	    FCL_KeyWord = FindFCLKeywordFromMap( *sci, false);
	    if ( FCL_KeyWord ) {
	      // Found FCL keyword, should be IS, AND, OR, NOT
	      if ( FCL_KeyWord != keyword_And and FCL_KeyWord != keyword_Or and 
		   FCL_KeyWord != keyword_IS  and FCL_KeyWord != keyword_Not ) {
		ErrMsg("Invalid keyword found in rule subCondition",
		       FCL_KeyWord->keyword + " in [" + FCLFileLine + "]", 1);
	      }
	      if ( FCL_KeyWord != keyword_IS and FCL_KeyWord != keyword_Not ) {
		Combination_FCL_KeyWord = FCL_KeyWord;
	      }
	    }
	    else if ( InputVariables.find(*sci) != InputVariables.end() ) {
	      // Found input variable
	      inputVariable = InputVariables[*sci];
	      variableName = *sci;

	      // Check to see if NOT was preceeding this variable
	      // This is for cases of: IF NOT (temp is hot) where the 
	      // condition is negated, not just the term (hot)
	      if ( sci != subCondWords.begin() ) {
		if ( *(sci - 1) == keyword_Not->keyword ) {
		  conditionNegate = true;
		}
	      }
	    }
	    else if ( inputVariable ) {
	      if ( inputVariable->InputTerms.find( *sci ) != 
		   inputVariable->InputTerms.end() ) {
		// Found inputTerm for inputVariable
		inputTerm = inputVariable->InputTerms[*sci];
		termName = *sci;
		// Check to see if NOT was preceeding this term
		if ( *(sci - 1) == keyword_Not->keyword ) {
		    termNegate = true;
		}
	      }
	      else {
		status = -1;
		ErrMsg("ParseFCL_IO_Rules() Failed find Rule term",
		       *sci + " in [" + FCLFileLine + "]", status);
		delete frc; delete condition;
		DebugAllMsg("<-ParseFCL_Rules()","", status);
		return status;
	      }
	    }
	    else {
	      status = -1;
	      ErrMsg("ParseFCL_IO_Rules() Failed find Rule item",
		     *sci + " in [" + FCLFileLine + "]", status);
	      delete frc; delete condition;
	      DebugAllMsg("<-ParseFCL_Rules()","", status);
	      return status;
	    }
	    // If found inputVar/inputTerm pair, then create subCondition
	    if ( inputVariable and inputTerm ) {
	      // Create a SubCondition struct
	      SubCondition* subCondition = new SubCondition;
	      if ( not subCondition ) {
		status = -1; delete frc; delete condition;
		ErrMsg("ParseFCL_IO_Rules() Failed to create "
		       "subCondition struct for rule",
		       ruleName + " in [" + FCLFileLine + "]", status);
		DebugAllMsg("<-ParseFCL_Rules()","", status);
		return status;
	      }
	      DebugAllMsg("Created SubCondition", variableName + 
			  " IS " + termName, 0);

	      // Assign the input variable/term to the subCondition struct
	      subCondition->inputVariable    = inputVariable;
	      subCondition->inputFuzzifyTerm = inputTerm;
	      subCondition->notTerm          = termNegate;
	      subCondition->notCondition     = conditionNegate;
	      subCondition->subResult        = 0.;

	      // Assign the subCondition struct to the AND/OR Condition stack
	      if ( not Combination_FCL_KeyWord ) {
		// This is the first (or only) variable/term set.
		// If it's the only one, then it goes into AND_SubConditions
		// If there are more, then it depends on the following 
		// Combination_FCL_KeyWord
		if ( conditionString.find(keyword_Or->keyword) != 
		     conditionString.npos ) {
		  // first variable/term followed by OR
		  condition->OR_SubConditions.push_back(subCondition);
		  DebugAllMsg("First SubCondition OR", variableName + 
			      " IS " + termName, 0);
		}
		else if ( conditionString.find(keyword_And->keyword) != 
			  conditionString.npos ) {
		  // first variable/term followed by AND
		  condition->AND_SubConditions.push_back(subCondition);
		  DebugAllMsg("First SubCondition AND", variableName + 
			      " IS " + termName, 0);
		}
		else {
		  // only subcondition
		  condition->AND_SubConditions.push_back(subCondition);
		  DebugAllMsg("Only SubCondition AND", variableName + 
			      " IS " + termName, 0);
		}
	      }
	      else if ( Combination_FCL_KeyWord == keyword_Or ) {
		// multiple variable/term preceeded by OR
		condition->OR_SubConditions.push_back(subCondition);
		DebugAllMsg("Multiple SubCondition OR", variableName + 
			    " IS " + termName, 0);
	      }
	      else if ( Combination_FCL_KeyWord == keyword_And ) {
		// multiple variable/term preceeded by AND
		condition->AND_SubConditions.push_back(subCondition);
		DebugAllMsg("Multiple SubCondition AND", variableName + 
			    " IS " + termName, 0);
	      }

	      inputVariable = 0; inputTerm = 0; Combination_FCL_KeyWord = 0;
	      termNegate = false; conditionNegate = false;
	    } // If found inputVar/inputTerm pair
	    
	    sci++;
	  } // while ( sci != subCondWords.end() ) 

	  // Assign the Condition struct to the FuzzyRuleClass->Conditions
	  frc->Conditions.push_back(condition);

	  // Read Conclusions
	  // Isolate the conclusions into a local string 
	  string conclusionString;
	  string::size_type endConclusionPosition = 0;

	  ThenStartPosition = FCLFileLine.find(keyword_Then->keyword);
	  if ( ThenStartPosition == FCLFileLine.npos ) {
	    status = -1; delete frc; delete condition;
	    ErrMsg("Failed to find THEN in rule conclusion",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  ThenStartPosition = 
	    FindNoWhiteSpace( FCLFileLine, 
			      ThenStartPosition + 
			      keyword_Then->keyword.length(), forward );

	  endConclusionPosition = FCLFileLine.find(";");
	  if ( endConclusionPosition == FCLFileLine.npos ) {
	    status = -1; delete frc; delete condition;
	    ErrMsg("Failed to find end-of-line delimeter ';' for rule",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  endConclusionPosition = FindNoWhiteSpace( FCLFileLine, 
						    --endConclusionPosition, 
						    back );
	  if ( endConclusionPosition <= ThenStartPosition ) {
	    status = -1;
	    ErrMsg("Failed to find conclusion in rule",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }
	  // conclusions are between THEN .... ;
	  conclusionString = FCLFileLine.substr( ThenStartPosition, 
						 endConclusionPosition - 
						 ThenStartPosition + 1);
	  DebugAllMsg("Conclusion", conclusionString, 0);
	  
	  // Find the outputVariables and the outputDefuzzifyTerms
	  // First, split the condition string into words
	  vector<string> subConclusionWords;
	  status = SplitLine( &subConclusionWords, &conclusionString, 
			      &delimeters );
	  if ( status != 0 ) {
	    ErrMsg("Failed to split rule conclusion",
		   ruleName + " in [" + FCLFileLine + "]", status);
	    delete frc; delete condition;
	    DebugAllMsg("<-ParseFCL_Rules()","", status);
	    return status;
	  }

	  // Iterate through the subConclusionWords, 
	  // creating the Conclusion structs,
	  sci = subConclusionWords.begin();
	  FuzzyOutputClass* outputVariable = 0;
	  FuzzyOutputTerm*  outputTerm     = 0;
	  double            outputScale    = 1.; // default is weight = 1. 
	  Conclusion*       lpConclusion   = 0;

	  while ( sci != subConclusionWords.end() ) {
	    FCL_KeyWord = FindFCLKeywordFromMap( *sci, false);
	    if ( FCL_KeyWord ) {
	      // Found FCL keyword, should be IS, WITH
	      if ( FCL_KeyWord != keyword_IS and FCL_KeyWord != keyword_With ) {
		ErrMsg("Invalid keyword found in rule Conclusion",
		       FCL_KeyWord->keyword + " in [" + FCLFileLine + "]", 1);
	      }
	    }
	    else if ( OutputVariables.find(*sci) != OutputVariables.end() ) {
	      // Found output variable
	      outputVariable = OutputVariables[*sci];
	      variableName = *sci;
	    }
	    else if ( outputVariable ) {
	      if ( outputVariable->OutputTerms.find(*sci) != 
		   outputVariable->OutputTerms.end() ) {
		// Found outputTerm for outputVariable
		outputTerm = outputVariable->OutputTerms[*sci];
		termName = *sci;
	      }
	    }
	    else if ( *(sci-1) == keyword_With->keyword ) {
	      // WITH was the preceeding keyword, so this should be a number
	      outputScale = atof ( sci->c_str() );
	      // assumes that the WITH follows a valid creation 
	      // of output/defuzzTerm pair
	      if ( lpConclusion ) {
		lpConclusion->weight = outputScale;
	      }
	      else {
		status = -1; delete frc; delete condition;
		ErrMsg("ParseFCL_IO_Rules() Failed to find Conclusion for WITH",
		       ruleName + " in [" + FCLFileLine + "]", status);
		DebugAllMsg("<-ParseFCL_Rules()","", status);
		return status;
	      }
	      outputScale = 1.;
	      DebugAllMsg("WITH", outputScale, 0);
	    }
	    else {
	      status = -1;
	      ErrMsg("ParseFCL_IO_Rules() Failed to find Rule item",
		     *sci + " in [" + FCLFileLine + "]", status);
	      delete frc; delete condition;
	      DebugAllMsg("<-ParseFCL_Rules()","", status);
	      return status;
	    }
	    // If found outputVar/outputTerm pair, then create Conclusion
	    if ( outputVariable and outputTerm ) {
	      // Create a Conclusion struct
	      lpConclusion = new Conclusion;
	      if ( not lpConclusion ) {
		status = -1; delete frc; delete condition;
		ErrMsg("ParseFCL_IO_Rules() Failed to create "
		       "Conclusion struct for rule",
		       ruleName + " in [" + FCLFileLine + "]", status);
		DebugAllMsg("<-ParseFCL_Rules()","", status);
		return status;
	      }
	      DebugAllMsg("Created Conclusion", variableName + 
			  " IS " + termName, 0);

	      // Assign the output variable/term to the subConclusion struct
	      lpConclusion->outputVariable  = outputVariable;
	      lpConclusion->outputDefuzzify = outputTerm;
	      lpConclusion->weight          = 1.; // default weight is 1.
	      // Assign the activationTerm varName and termName
	      lpConclusion->activationTerm.varName  = outputVariable->varName;
	      lpConclusion->activationTerm.termName = outputTerm->termName;
	      lpConclusion->activationTerm.termType = -1;
	      lpConclusion->activationTerm.singleton.x = 0.;
	      lpConclusion->activationTerm.singleton.y = 0.;

	      // Assign the Conclusion struct to the FuzzyRuleClass->Conclusions
	      frc->Conclusions.push_back(lpConclusion);

	      outputVariable = 0; outputTerm = 0;
	    } // if ( outputVariable and outputTerm )

	    sci++;
	  } // while ( sci != subConclusionWords.end() )

	  // Assign the FuzzyRuleClass to the Rules map
	  if ( Rules.find(ruleName) != Rules.end() ) {
	    status = -1;
	    ErrMsg("Found redundant rule definition", ruleName, status);
	    return status;
	  }
	  Rules[ruleName] = frc;

	} // Found RULE

      } // iterate until END_RULEBLOCK

    } // found RULEBLOCK

    FCLFileIterator++;
  } // while (FCLFileIterator != FCLFileVector.end())

  DebugAllMsg("<-ParseFCL_Rules()","", status);
  return status;
}

//--------------------------------------------------------------
// ParseFCL_Output_Defuzzify
//
// Purpose: Parse the Output Defuzzify Block
//
// Arguments:
//           
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCL_Output_Defuzzify() {

  status = 0;
  DebugAllMsg("->ParseFCL_Output_Defuzzify()","", status);

  string FCLFileLine;
  string::size_type stringKeyStart;
  string::size_type stringKeyEnd;
  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();
  FCLFileIterator = FCLFileVector.begin();

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileLine = *FCLFileIterator;

    // Look for DEFUZZIFY
    stringKeyStart = FCLFileLine.find(keyword_Defuzzify->keyword);
    if ( stringKeyStart != FCLFileLine.npos ) {
      // found DEFUZZIFY  Get the input variable name
      string variableName;
      string termName;
      string::size_type startVarNamePosition;
      string::size_type endVarNamePosition;
      string::size_type RangeStartPosition;

      // Find start of variable name
      startVarNamePosition = 
	FindNoWhiteSpace( FCLFileLine,
			  stringKeyStart + keyword_Defuzzify->keyword.length(),
			  forward );
      // Find end of variable name
      endVarNamePosition = FCLFileLine.length();
      endVarNamePosition = FindNoWhiteSpace( FCLFileLine, 
					     endVarNamePosition, back );
      if ( endVarNamePosition <= startVarNamePosition ) {
	status = -1;
	ErrMsg("Failed to find end of DEFUZZIFY output variable name",
	       " in [" + FCLFileLine + "]", status);
	DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	return status;
      }
      // Assign the variable name string
      variableName = FCLFileLine.substr(startVarNamePosition, 
					endVarNamePosition - 
					startVarNamePosition + 1);
  
      DebugAllMsg("Found DEFUZZIFY Variable ",  variableName, status);

      // Find the output variable in the OutputVariables map, 
      // the FuzzyOutputTerm is inserted into the FuzzyOutputClass 
      // after parsing/creation
      if ( OutputVariables.find(variableName) == OutputVariables.end() ) {
	status = -1;
	ErrMsg("Failed to find FuzzyOutput variable in "
	       "OutputVariables database ",
	       variableName + " in [" + FCLFileLine + "]", status);
	DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	return status;
      }

      // iterate until END_DEFUZZIFY, create output variable terms, 
      // get term values
      while (FCLFileIterator != FCLFileVector.end()) {
	string::size_type TermStartPosition;
	string::size_type AccuStartPosition;
	string::size_type MethodStartPosition;
	string::size_type DefaultStartPosition;

	FCLFileIterator++;
	FCLFileLine = *FCLFileIterator;

	// Look for END_DEFUZZIFY 
	stringKeyEnd = FCLFileLine.find(keyword_EndDefuzzify->keyword);
	if ( stringKeyEnd != FCLFileLine.npos ) {
	  // found END_DEFUZZIFY, break to continue looking for 
	  // next DEFUZZIFY block
	  break;
	}

	// Look for TERM, ACCU, METHOD, DEFAULT, RANGE
	AccuStartPosition    = FCLFileLine.find(keyword_Accu->keyword);
	MethodStartPosition  = FCLFileLine.find(keyword_Method->keyword);
	DefaultStartPosition = FCLFileLine.find(keyword_Default->keyword);
	TermStartPosition    = FCLFileLine.find(keyword_Term->keyword);
	RangeStartPosition   = FCLFileLine.find(keyword_Range->keyword);

	if ( TermStartPosition != FCLFileLine.npos ) {
	  // Found TERM, get name, value
	  string::size_type startTermNamePosition;
	  string::size_type endTermNamePosition;
	  string::size_type startTermValuePosition;

	  // Find start of the term name
	  startTermNamePosition = 
	    FindNoWhiteSpace( FCLFileLine,
			      TermStartPosition + 
			      keyword_Term->keyword.length(),
			      forward );
	  // Find end of term name
	  endTermNamePosition = FCLFileLine.find(":=");
	  // In case the defuzz is a singleton, set the startTermValuePosition
	  startTermValuePosition = FindNoWhiteSpace( FCLFileLine, 
						     endTermNamePosition + 2, 
						     forward);
	  endTermNamePosition = 
	    FindNoWhiteSpace( FCLFileLine, --endTermNamePosition, back );

	  if ( endTermNamePosition <= startTermNamePosition or 
	       ( endTermNamePosition == FCLFileLine.npos ) ) {
	    status = -1;
	    ErrMsg("Failed to find end of DEFUZZIFY variable name",
		   " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  // Assign the variable name string
	  termName = FCLFileLine.substr(startTermNamePosition, 
					endTermNamePosition - 
					startTermNamePosition + 1);
  
	  DebugAllMsg("Found TERM ", termName, status);

	  // Create the FuzzyOutputTerm
	  FuzzyOutputTerm* fot = new FuzzyOutputTerm;
	  FuzzyOutputTerm* fat = new FuzzyOutputTerm; // accumulation term
	  if ( not fot or not fat ) {
	    status = -1;
	    ErrMsg("Failed to create FuzzyOutputTerm for", variableName,status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }

	  DebugAllMsg("Created FuzzyOutputTerm for", termName, status);

	  fot->varName      = variableName;
	  fot->termName     = termName;
	  fot->termType     = -1;
	  fot->singleton.x  = 0.;
	  fot->singleton.y  = 0.;
	  fat->varName      = variableName;
	  fat->termName     = termName;
	  fat->termType     = -1;
	  fat->singleton.x  = 0.;
	  fat->singleton.y  = 0.;

	  string::size_type endTermLinePosition = 0;
	  endTermLinePosition = FCLFileLine.find_first_of(";");
	  if ( endTermLinePosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find term end of line delimeter ';' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    delete fot; delete fat;
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }

	  // Get the TERM, can be Singleton or Membership Function
	  string::size_type termTypePosition;
	  termTypePosition = FCLFileLine.find("(");

	  if ( termTypePosition == FCLFileLine.npos or 
	       endTermLinePosition < termTypePosition ) {
	    // The Defuzzification is a Singleton
	    fot->termType = Singleton;
	    fat->termType = Singleton;
	    // Get the output value of the term
	    fot->singleton.x = atof( &FCLFileLine[startTermValuePosition] );

	    DebugAllMsg("Found output term Singleton value", 
			fot->singleton.x, status);
	  }
	  else {
	    // It's a membership function TERM
	    string::size_type startTermValuePosition = 0;
	    string::size_type endTermValuePosition   = 0;
	    string::size_type endTermXPosition       = 0;
	    string::size_type currentEndTermPosition = 0;
	    int numberOfTermPoints = 0;

	    // Validate the x.y pair delimeters ()
	    status = CheckFuzzyTermParen( &FCLFileLine );
	    if ( status != 0 ) {
	      ErrMsg("Failed to find valid DEFUZZIFY term x,y "
		     "value delimeters '()' for term",
		     termName + " in [" + FCLFileLine + "]", status);
	      DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	      return status;
	    }

	    // Iterate on the points, each point is of the 
	    // format := (x1,y1) (x2,y2);
	    while ( currentEndTermPosition < endTermLinePosition ) {
	      // Increment the position vars since the X.Y pairs 
	      // don't start the line that way it will work for 
	      // iterating through the line to find subsequent X.Y pairs
	      startTermValuePosition = 
		FCLFileLine.find_first_of("(", ++startTermValuePosition);

	      endTermValuePosition = 
		FCLFileLine.find_first_of(")", ++endTermValuePosition );

	      if ( startTermValuePosition == FCLFileLine.npos or 
		   endTermValuePosition   == FCLFileLine.npos ) {
		status = -1;
		ErrMsg("Failed to find DEFUZZIFY term value "
		       "delimeters for term",
		       termName + " in [" + FCLFileLine + "]", status);
		delete fot; delete fat;
		DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
		return status;
	      }
	      endTermXPosition = 
		FCLFileLine.find_first_of(",", startTermValuePosition + 1);
	      if ( endTermXPosition == FCLFileLine.npos or 
		   endTermXPosition >= endTermValuePosition ) {
		status = -1;
		ErrMsg("Failed to find DEFUZZIFY term value x,y "
		       "delimeter ',' for term",
		       termName + " in [" + FCLFileLine + "]", status);
		delete fot; delete fat;
		DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
		return status;
	      }
	      // Get the X,Y values
	      double X = 0.;
	      double Y = 0.;
	      X = atof( &FCLFileLine[startTermValuePosition + 1] );
	      Y = atof( &FCLFileLine[endTermXPosition + 1] );

	      // Create the XY struct
	      XY *lpTermXY = new XY;
	      if ( not lpTermXY ) {
		status = -1;
		ErrMsg("Failed to create XY struct for", termName, status);
		delete fot; delete fat;
		DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
		return status;
	      }
	      lpTermXY->x = X;
	      lpTermXY->y = Y;

	      // insert XY into the FuzzyOutputTerm
	      fot->xy.push_back(lpTermXY);

	      DebugAllMsg("Created X value", 
			  fot->xy[numberOfTermPoints]->x, status);
	      DebugAllMsg("Created Y value", 
			  fot->xy[numberOfTermPoints]->y, status);
	  
	      numberOfTermPoints++;

	      // Look for end of line
	      currentEndTermPosition = 
		FindNoWhiteSpace( FCLFileLine, 
				  endTermValuePosition + 1, forward );
	      if ( currentEndTermPosition == endTermLinePosition ) {
		break;
	      }
	    } // while ( currentEndTermPosition < endTermLinePosition ) 
	      // getting XY

	    // Classify the type of term
	    switch (numberOfTermPoints) {
	    case 1:
	      fot->termType = Singleton;
	      fat->termType = Singleton;
	      break;
	    case 2:
	      fot->termType = Ramp;
	      fat->termType = Ramp;
	      break;
	    case 3:
	      fot->termType = Triangle;
	      fat->termType = Triangle;
	      break;
	    case 4:
	      if ( fot->xy[0]->x == fot->xy[1]->x and 
		   fot->xy[2]->x == fot->xy[3]->x ) {
		fot->termType = Rectangle;
		fat->termType = Rectangle;
	      }
	      else if ( fot->xy[0]->x != fot->xy[1]->x and 
			fot->xy[2]->x != fot->xy[3]->x ) {
		fot->termType = Trapezoid;
		fat->termType = Trapezoid;
	      }
	      else {
		ErrMsg("Failed to classify DEFUZZIFY term" ,
		       termName + " in [" + FCLFileLine + "]", status);
		delete fot; delete fat;
		DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
		return status;
	      }
	      break;
	    default:
	      status = -1;
	      ErrMsg("Failed to classify DEFUZZIFY term" ,
		     termName + " in [" + FCLFileLine + "]", status);
	      delete fot; delete fat;
	      DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	      return status;
	    };

	    DebugAllMsg("Found points in term", numberOfTermPoints, status);
	  }

	  // Assign the FuzzyOutputTerm to the OutputTerms map in 
	  // the FuzzyOutputClass
	  OutputVariables[variableName]->OutputTerms[termName] = fot;
	  OutputVariables[variableName]->AccumulationTerms[termName] = fat;

	} // Look for TERM, get name, value

	else if ( AccuStartPosition != FCLFileLine.npos ) {
	  // Found ACCU, get accumulation type, either MAX, BSUM, NSUM
	  string accumulation;
	  string::size_type startPosition;
	  string::size_type endPosition;
	  startPosition = FCLFileLine.find(":");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find ACCU end delimeter ':' for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  endPosition = FCLFileLine.find(";");
	  if ( endPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find ACCU end of line delimeter ';' for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  startPosition = FindNoWhiteSpace( FCLFileLine, 
					    ++startPosition, forward);
	  endPosition   = FindNoWhiteSpace( FCLFileLine, 
					    --endPosition,   back);
	  if ( endPosition <= startPosition ) {
	    status = -1;
	    ErrMsg("Failed to find ACCU method for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  accumulation = FCLFileLine.substr( startPosition, 
					     endPosition - startPosition + 1);
	  // Make sure the method is valid: MAX, BSUM, NSUM
	  if ( accumulation != keyword_Max->keyword and 
	       accumulation != keyword_Bsum->keyword and
	       accumulation != keyword_Nsum->keyword ) {
	    status = -1;
	    ErrMsg("Invalid ACCU method in output for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;	    
	  }
	  // Find the output variable in the map
	  if ( OutputVariables.find(variableName) == OutputVariables.end() ) {
	    status = -1;
	    ErrMsg("Failed to find variable in OutputVariables database",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  if ( OutputVariables[variableName]->OutputTerms.find(termName) == 
	       OutputVariables[variableName]->OutputTerms.end() ) {
	    status = -1;
	    ErrMsg("Failed to find FuzzyOutputTerm in OutputTerms database",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  // Assign the FCL_keyword for ACCU method to the 
	  // FuzzyOutputClass.accumulation
	  FCL_keyword* accuMethod = FindFCLKeywordFromMap(accumulation, true);
	  if ( not accuMethod ) {
	    status = -1;
	    ErrMsg("Failed to find keyword for FuzzyOutput variable" ,
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  OutputVariables[variableName]->accumulation = accuMethod;
	  
	  DebugAllMsg("Created ACCU method", accuMethod->keyword, status);
	  DebugAllMsg("ACCU variable" , variableName, status);
	}

	else if ( MethodStartPosition != FCLFileLine.npos ) {
	  // Found METHOD, get defuzzification method: COA, COG, COGS, LM, RM
	  string method;
	  string::size_type startPosition;
	  string::size_type endPosition;
	  startPosition = FCLFileLine.find(":");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find METHOD end delimeter ':' for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  endPosition = FCLFileLine.find(";");
	  if ( endPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find METHOD end of line delimeter ';' "
		   "for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  startPosition = FindNoWhiteSpace( FCLFileLine, 
					    ++startPosition, forward);
	  endPosition   = FindNoWhiteSpace( FCLFileLine, 
					    --endPosition,   back);
	  if ( endPosition <= startPosition ) {
	    status = -1;
	    ErrMsg("Failed to find METHOD method for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  method = FCLFileLine.substr( startPosition, 
				       endPosition - startPosition + 1);
	  // Make sure the method is valid: COA, COG, COGS, LM, RM
	  if ( method != keyword_COA->keyword  and 
	       method != keyword_COG->keyword  and
	       method != keyword_COGS->keyword and 
	       method != keyword_LM->keyword   and 
	       method != keyword_RM->keyword ) {
	    status = -1;
	    ErrMsg("Invalid METHOD in output for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;	    
	  }
	  // find the output variable in the OutputVariables map
	  if ( OutputVariables.find(variableName) == OutputVariables.end() ) {
	    status = -1;
	    ErrMsg("Failed to find variable in OutputVariables database",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  if ( OutputVariables[variableName]->OutputTerms.find(termName) == 
	       OutputVariables[variableName]->OutputTerms.end() ) {
	    status = -1;
	    ErrMsg("Failed to find FuzzyOutputTerm in OutputTerms database",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  // Assign the FCL_keyword for defuzzify METHOD to the 
	  // FuzzyOutputClass.method
	  FCL_keyword* defuzzMethod = FindFCLKeywordFromMap(method, true);
	  if ( not defuzzMethod ) {
	    status = -1;
	    ErrMsg("Failed to find keyword in keywords map for "
		   "FuzzyOutput variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  OutputVariables[variableName]->method = defuzzMethod;
	  
	  DebugAllMsg("Created defuzzification METHOD", 
		      defuzzMethod->keyword, status);
	  DebugAllMsg("METHOD variable" , variableName, status);
	}

	else if ( DefaultStartPosition != FCLFileLine.npos ) {
	  // Found DEFAULT, either a real value, or NC for no change
	  string defaultOutput;
	  string::size_type startPosition;
	  string::size_type endPosition;
	  startPosition = FCLFileLine.find(":=");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find DEFAULT end delimeter ':=' for variable" ,
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  endPosition = FCLFileLine.find(";");
	  if ( endPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find DEFAULT end of line delimeter ';' "
		   "for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  startPosition = FindNoWhiteSpace( FCLFileLine, 
					    startPosition + 2, forward );
	  endPosition   = FindNoWhiteSpace( FCLFileLine, 
					    endPosition - 1,   back );
	  if ( endPosition < startPosition ) { 
	    // could be equal if single digit i.e. '0'
	    status = -1;
	    ErrMsg("Failed to find DEFAULT for variable",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  // This will be either "NC" keyword or a value
	  defaultOutput = FCLFileLine.substr( startPosition, 
					      endPosition - startPosition + 1);

	  if ( OutputVariables.find(variableName) == OutputVariables.end() ) {
	    status = -1;
	    ErrMsg("Failed to find variable in OutputVariables database",
		   variableName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  if ( OutputVariables[variableName]->OutputTerms.find(termName) == 
	       OutputVariables[variableName]->OutputTerms.end() ) {
	    status = -1;
	    ErrMsg("Failed to find FuzzyOutputTerm in OutputTerms database",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }

	  // Assign the FCL_keyword for defuzzify DEFAULT to the 
	  // FuzzyOutputClass.method
	  // if the value "NC" was set, then set it to NC, 
	  // No Change in output value.
	  // Otherwise, set the DefaultOut variable in the FuzzyOutputClass
	  if ( defaultOutput == keyword_NC->keyword ) {
	    OutputVariables[variableName]->defaultNC = keyword_NC;
	    DebugAllMsg("Created defuzzification DEFAULT", 
			keyword_NC->keyword, status);
	  }
	  else {
	    // DEFAULT wasn't set to NC, so it must be a value
	    OutputVariables[variableName]->defaultOut = 
	      atof( &FCLFileLine[startPosition] );

	    DebugAllMsg("Created defuzzification DEFAULT", 
			OutputVariables[variableName]->defaultOut, status);
	  }
	  
	  DebugAllMsg("DEFAULT variable", variableName, status);
	}

	else if ( RangeStartPosition != FCLFileLine.npos ) {
	  // Found RANGE
	  string::size_type startPosition;
	  startPosition = FCLFileLine.find(":=");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find range delimeter ':=' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  startPosition = FCLFileLine.find("(");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find range start delimeter '(' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  double minOut = 0.;
	  double maxOut = 0.;

	  startPosition = FindNoWhiteSpace( FCLFileLine, 
					    ++startPosition, forward );
	  minOut = atof( &FCLFileLine[startPosition] );

	  startPosition = FCLFileLine.find(",");
	  if ( startPosition == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find range separation delimeter ',' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
	    return status;
	  }
	  startPosition = FindNoWhiteSpace( FCLFileLine, 
					    ++startPosition, forward );
	  maxOut = atof( &FCLFileLine[startPosition] );
	  
	  OutputVariables[variableName]->minOut = minOut;
	  OutputVariables[variableName]->maxOut = maxOut;
	}

      } // iterate until END_DEFUZZIFY, create input variable terms

      if ( RangeStartPosition == FCLFileLine.npos ) {
	// Did not find RANGE
	status = -1;
	ErrMsg("No RANGE defined for output variable", variableName, status);
	return status;
      }
    } // if ( stringKeyStart != FCLFileLine.npos ) // found DEFUZZIFY

    FCLFileIterator++;
  } // while (FCLFileIterator != FCLFileVector.end())

  DebugAllMsg("<-ParseFCL_Output_Defuzzify()","", status);
  return status;
}

//--------------------------------------------------------------
// ParseFCL_Input_Fuzzify
//
// Purpose: Parse the Input Fuzzify Block
//
// Arguments:
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCL_Input_Fuzzify() {

  status = 0;
  DebugAllMsg("->ParseFCL_Input_Fuzzify()","", status);

  string FCLFileLine;
  string::size_type stringKeyStart;
  string::size_type stringKeyEnd;
  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();
  FCLFileIterator = FCLFileVector.begin();

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileLine = *FCLFileIterator;
    
    // Find the FUZZIFY in the FCL file data
    stringKeyEnd = FCLFileLine.find(keyword_EndFuzzify->keyword);
    if ( stringKeyEnd != FCLFileLine.npos ) {
      // found END_FUZZIFY
      FCLFileIterator++; // continue looking for next FUZZIFY block
      continue;
    }
    stringKeyEnd = FCLFileLine.find(keyword_Defuzzify->keyword);
    if ( stringKeyEnd != FCLFileLine.npos ) {
      // found DEFUZZIFY or END_DEFUZZIFY
      FCLFileIterator++;
      continue;
    }
    stringKeyStart = FCLFileLine.find(keyword_Fuzzify->keyword);
    if ( stringKeyStart != FCLFileLine.npos ) {
      // found FUZZIFY
      // Get the input variable name
      string variableName;
      string::size_type startVarNamePosition;
      string::size_type endVarNamePosition;

      // Find start of variable name, FUZZIFY is 7 chars long
      startVarNamePosition = 
	FindNoWhiteSpace( FCLFileLine,
			  stringKeyStart + keyword_Fuzzify->keyword.length(),
			  forward );

      // Find end of variable name
      endVarNamePosition = FCLFileLine.length();
      endVarNamePosition = FindNoWhiteSpace( FCLFileLine, 
					     --endVarNamePosition, back );

      if ( endVarNamePosition <= startVarNamePosition ) {
	status = -1;
	ErrMsg("Failed to find end of FUZZIFY input variable name",
	       "in [" + FCLFileLine + "]", status);
	DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	return status;
      }

      // Assign the variable name string
      variableName = FCLFileLine.substr(startVarNamePosition, 
					endVarNamePosition - 
					startVarNamePosition + 1);

      DebugAllMsg("Found FUZZIFY Variable ",  variableName, status);

      // Find the input variable in the InputVariables map, 
      // the FuzzyInputTerm is inserted into the FuzzyInputClass 
      // after parsing/creation
      if ( InputVariables.find(variableName) == InputVariables.end() ) {
	status = -1;
	ErrMsg("Failed to find FuzzyInput variable in InputVariables "
	       "database, variable name ", 
	       variableName, status);
	ErrMsg("in [", FCLFileLine + "]", status);
	DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	return status;
      }

      // iterate until END_FUZZIFY, create input variable terms
      while (FCLFileIterator != FCLFileVector.end()) {
	string termName;
	string::size_type TermStartPosition;
	string::size_type startTermNamePosition;
	string::size_type endTermNamePosition;

	FCLFileIterator++;
	FCLFileLine = *FCLFileIterator;

	// Find start of TERM in string
	TermStartPosition = FCLFileLine.find(keyword_Term->keyword);
	if ( TermStartPosition == FCLFileLine.npos ) {
	  break; // line with no TERM
	}
	// Find start of the term name, TERM is 4 chars long
	startTermNamePosition = 
	  FindNoWhiteSpace( FCLFileLine,
			    TermStartPosition + keyword_Term->keyword.length(),
			    forward );
	// Find end of term name
	endTermNamePosition = FCLFileLine.find(":=");
	endTermNamePosition = FindNoWhiteSpace( FCLFileLine, 
						--endTermNamePosition, back );
	if ( endTermNamePosition <= startTermNamePosition or 
	     ( endTermNamePosition == FCLFileLine.npos ) ) {
	  status = -1;
	  ErrMsg("Failed to find end of FUZZIFY term input name",
		 "in [" + FCLFileLine + "]", status);
	  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	  return status;
	}
	// Assign the variable name string
	termName = FCLFileLine.substr(startTermNamePosition, 
				      endTermNamePosition - 
				      startTermNamePosition + 1);
  
	DebugAllMsg("Found TERM ", termName, status);

	// Check for end of line delimeter ';'
	string::size_type endTermLinePosition = 0;
	endTermLinePosition = FCLFileLine.find_first_of(";");
	if ( endTermLinePosition == FCLFileLine.npos ) {
	  status = -1;
	  ErrMsg("Failed to find term end of line delimeter ';' for term",
		 termName + " in [" + FCLFileLine + "]", status);
	  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	  return status;
	}

	// Create the FuzzyInputTerm
	FuzzyInputTerm* fit = new FuzzyInputTerm;
	if ( not fit ) {
	  status = -1;
	  ErrMsg("Failed to create FuzzyInputTerm for ", variableName, status);
	  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	  return status;
	}

	DebugAllMsg("Created FuzzyInputTerm for", termName, status);

	fit->varName    = variableName;
	fit->termName   = termName;
	fit->membership = 0;
	fit->termType   = -1;

	//
	// Get the membership func x,y pairs of the term
	//
	string::size_type startTermValuePosition = 0;
	string::size_type endTermValuePosition   = 0;
	string::size_type endTermXPosition       = 0;
	string::size_type currentEndTermPosition = 0;
	int numberOfTermPoints = 0;

	// Validate the x.y pair delimeters ()
	status = CheckFuzzyTermParen( &FCLFileLine );
	if ( status != 0 ) {
	  ErrMsg("Failed to find valid FUZZIFY term x,y value "
		 "delimeters '()' for term",
		 termName + " in [" + FCLFileLine + "]", status);
	  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	  return status;
	}

	// Iterate on the points, each point is of the 
	// format := (x1,y1) (x2,y2);
	while ( currentEndTermPosition < endTermLinePosition ) {
	  // Increment the position vars since the X.Y pairs 
	  // don't start the line that way it will work for 
	  // iterating through the line to find subsequent X.Y pairs
	  startTermValuePosition = 
	    FCLFileLine.find_first_of("(", ++startTermValuePosition);

	  endTermValuePosition = 
	    FCLFileLine.find_first_of(")", ++endTermValuePosition );

	  if ( startTermValuePosition == FCLFileLine.npos or 
	       endTermValuePosition   == FCLFileLine.npos ) {
	    status = -1;
	    ErrMsg("Failed to find term value delimeters '()' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    delete fit;
	    DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	    return status;
	  }
	  endTermXPosition = 
	    FCLFileLine.find_first_of(",", startTermValuePosition + 1);

	  if ( endTermXPosition == FCLFileLine.npos or 
	       endTermXPosition >= endTermValuePosition ) {
	    status = -1;
	    ErrMsg("Failed to find term value x,y delimeter ',' for term",
		   termName + " in [" + FCLFileLine + "]", status);
	    delete fit;
	    DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	    return status;
	  }
	  // Get the X,Y values
	  double X = 0.;
	  double Y = 0.;
	  X = atof( &FCLFileLine[startTermValuePosition + 1] );
	  Y = atof( &FCLFileLine[endTermXPosition + 1] );

	  // Create the XY struct
	  XY *lpTermXY = new XY;
	  if ( not lpTermXY ) {
	    status = -1;
	    ErrMsg("Failed to create XY struct for ",
		   termName + " in [" + FCLFileLine + "]", status);
	    delete fit;
	    DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	    return status;
	  }
	  lpTermXY->x = X;
	  lpTermXY->y = Y;

	  // insert XY into the FuzzyInputTerm
	  fit->xy.push_back(lpTermXY);

	  DebugAllMsg("Created X value",fit->xy[numberOfTermPoints]->x, status);
	  DebugAllMsg("Created Y value",fit->xy[numberOfTermPoints]->y, status);
	  
	  numberOfTermPoints++;

	  // Look for end of line
	  currentEndTermPosition = 
	    FindNoWhiteSpace( FCLFileLine, endTermValuePosition + 1, forward );

	  if ( currentEndTermPosition == endTermLinePosition ) {
	    break;
	  }
	} // while ( currentEndTermPosition < endTermLinePosition ) 
	// getting XY

	// Classify the type of term
	switch (numberOfTermPoints) {
	case 1:
	  fit->termType = Singleton;
	  break;
	case 2:
	  fit->termType = Ramp;
	  break;
	case 3:
	  fit->termType = Triangle;
	  break;
	case 4:
	  if ( fit->xy[0]->x == fit->xy[1]->x and 
	       fit->xy[2]->x == fit->xy[3]->x ) {
	    fit->termType = Rectangle;
	  }
	  else if ( fit->xy[0]->x != fit->xy[1]->x and 
		    fit->xy[2]->x != fit->xy[3]->x ) {
	    fit->termType = Trapezoid;
	  }
	  else {
	    status = -1;
	    ErrMsg("Failed to classify term", termName + 
		   " in [" + FCLFileLine + "]", status);
	    delete fit;
	    DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	    return status;
	  }
	  break;
	case 0:
	default:
	  status = -1;
	  ErrMsg("Failed to classify term", termName + 
		 " in [" + FCLFileLine + "]", status);
	  delete fit;
	  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
	  return status;
	};

	DebugAllMsg("Found points in term", numberOfTermPoints, status);

	// Assign the FuzzyInputTerm to the InputTerms map in the 
	// FuzzyInputClass
	InputVariables[variableName]->InputTerms[termName] = fit;

      } // while (FCLFileIterator != FCLFileVector.end()) 
      // iterate until END_FUZZIFY
      
    } // if ( stringKeyStart != FCLFileLine.npos ) // found FUZZIFY

    FCLFileIterator++;
  } // while (FCLFileIterator != FCLFileVector.end())

  DebugAllMsg("<-ParseFCL_Input_Fuzzify()","", status);
  return status;
}

//--------------------------------------------------------------
// ParseFCL_IO_Vars
//
// Purpose: Parse the Input/Output Variables
//
// Arguments: pointer to FCL_keyword for start/end of block
//            keywordStart is either VAR_INPUT or VAR_OUTPUT
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCL_IO_Vars(FCL_keyword* keywordStart,
					FCL_keyword* keywordEnd) {

  status = 0;
  DebugAllMsg("->ParseFCL_IO_Vars()","", status);
  
  // Find the VAR_INPUT or VAR_OUTPUT in the FCL file data
  string FCLFileLine;
  string::size_type stringPosition;
  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();
  FCLFileIterator = FCLFileVector.begin();

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileLine = *FCLFileIterator;
    
    stringPosition = FCLFileLine.find(keywordStart->keyword);
    if ( stringPosition != FCLFileLine.npos ) {
      // found VAR_INPUT or VAR_OUTPUT 
      break;
    }
    FCLFileIterator++;
  }
  if ( FCLFileIterator == FCLFileVector.end() ) {
    status = -1;
    ErrMsg("Failed to find input/output variable definition in FCL file",
	   keywordStart->keyword, status);
    DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
    return status;
  }

  // Now iterate and process input/output variables until END_VAR is found
  string variableName;
  string variableType;
  string::size_type startVarNamePosition;
  string::size_type endVarNamePosition;
  string::size_type startVarTypePosition;
  string::size_type endVarTypePosition;

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileIterator++;
    FCLFileLine = *FCLFileIterator;

    // end loop when find next END_VAR
    stringPosition = FCLFileLine.find(keywordEnd->keyword);
    if ( stringPosition != FCLFileLine.npos ) {
      // found END_VAR
      break;
    }

    // Check for EOL delimeter ';'
    endVarTypePosition = FCLFileLine.find(';');
    if ( endVarTypePosition >= FCLFileLine.npos ) {
      status = -1;
      ErrMsg("Failed to find input/output variable end-of-line delimeter", 
	     "';' in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }

    // Get the variable name
    // Find first non-whitespace in string
    startVarNamePosition = FindNoWhiteSpace( FCLFileLine, 0, forward );
    if ( startVarNamePosition >= FCLFileLine.size() ) {
      status = -1;
      ErrMsg("Failed to find start of input/output variable name",
	     "in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }

    // Find end of variable name
    endVarNamePosition = FCLFileLine.find(":");
    if ( endVarNamePosition == FCLFileLine.npos ) {
      status = -1;
      ErrMsg("Failed to find input/output variable name end delimeter",
	     "':' in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }
    startVarTypePosition = endVarNamePosition + 1;
    endVarNamePosition = FindNoWhiteSpace( FCLFileLine, 
					   --endVarNamePosition, back );
    if ( endVarNamePosition <= startVarNamePosition ) {
      status = -1;
      ErrMsg("Failed to find end of input/output variable name",
	     "in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }
    // Assign the variable name string
    variableName = FCLFileLine.substr(startVarNamePosition, 
				      endVarNamePosition - 
				      startVarNamePosition + 1);
    
    // Get the variable type
    startVarTypePosition = FindNoWhiteSpace( FCLFileLine, 
					     startVarTypePosition , forward );
    if ( startVarTypePosition >= FCLFileLine.size() ) {
      status = -1;
      ErrMsg("Failed to find start of input/output variable type",
	     "in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }
    // Backup from endVarTypePosition to remove any whitespace past the type
    endVarTypePosition = FindNoWhiteSpace( FCLFileLine, 
					   --endVarTypePosition, back );
    if ( endVarTypePosition <= startVarTypePosition ) {
      status = -1;
      ErrMsg("Failed to find end of input/output variable type",
	     "in [" + FCLFileLine + "]", status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }
    // Assign the variable type string
    variableType = FCLFileLine.substr(startVarTypePosition,
				      endVarTypePosition - 
				      startVarTypePosition + 1);

    // Switch depending on whether it's input/output variable creation
    if (keywordStart == keyword_VarIn) {
      // Create the input variable object
      // First, create the FuzzyInputClass
      FuzzyInputClass* fic = new FuzzyInputClass;
      if ( not fic ) {
	status = -1;
	ErrMsg("ParseFCL_IO_Vars() Failed to create FuzzyInputClass for ", 
	       variableName, status);
	DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
	return status;
      }
      fic->varName = variableName;
      fic->varType = variableType;
      // Insert the new input variable into the InputVariables map
      InputVariables[variableName] = fic;

      DebugAllMsg("Created input variable", variableName, status);
    }
    else if (keywordStart == keyword_VarOut) {
      // Create the output variable object
      // First, create the FuzzyOutputClass, pass in default values
      FuzzyOutputClass* foc = new FuzzyOutputClass(keyword_Max, keyword_COGS);
      if ( not foc ) {
	status = -1;
	ErrMsg("ParseFCL_IO_Vars() Failed to create FuzzyOutputClass for ", 
	       variableName, status);
	DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
	return status;
      }
      foc->varName = variableName;
      foc->varType = variableType;
      // Insert the new output variable into the OutputVariables map
      OutputVariables[variableName] = foc;

      DebugAllMsg("Created output variable", variableName, status);
    }
    else {
      status = -1;
      ErrMsg("Invalid keyword for variable I/O definition" , 
	     keywordStart->keyword, status);
      DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
      return status;
    }
    DebugAllMsg("Variable type", variableType, status);

  } // while (FCLFileIterator != FCLFileVector.end())

  DebugAllMsg("<-ParseFCL_IO_Vars()","", status);
  return status;
}

//--------------------------------------------------------------
// ParseFCL_CheckFile
//
// Purpose: Check FCL file for variable blocks
//
// Arguments:
//           
// Return:   
//--------------------------------------------------------------
int FuzzyControlClass::ParseFCL_CheckFile() {

  status = 0;
  int iVarInput  = 0;
  int iVarOutput = 0;
  
  // Find the VAR_INPUT or VAR_OUTPUT in the FCL file data
  string FCLFileLine;
  string::size_type startVarInPosition;
  string::size_type startVarOutPosition;

  vector<string>::iterator FCLFileIterator = FCLFileVector.begin();
  FCLFileIterator = FCLFileVector.begin();

  while (FCLFileIterator != FCLFileVector.end()) {
    FCLFileLine = *FCLFileIterator;
    
    startVarInPosition = FCLFileLine.find(keyword_VarIn->keyword);
    startVarOutPosition = FCLFileLine.find(keyword_VarOut->keyword);
    if ( startVarInPosition != FCLFileLine.npos ) {
      // found VAR_INPUT
      iVarInput++;
    }
    if ( startVarOutPosition != FCLFileLine.npos ) {
      // found VAR_OUTPUT
      iVarOutput++;
    }
    FCLFileIterator++;
  }
  if ( not iVarInput ) {
    status = -1;
    ErrMsg("Failed to find input variable definition in FCL file" , "", status);
    return status;
  }
  if ( not iVarOutput ) {
    status = -1;
    ErrMsg("Failed to find output variable definition in FCL file" ,"", status);
    return status;
  }
  if ( iVarInput > 1 ) {
    status = -1;
    ErrMsg("Too many input variable blocks in FCL file" , "", status);
    return status;
  }
  if ( iVarOutput > 1 ) {
    status = -1;
    ErrMsg("Too many output variable blocks in FCL file" , "", status);
    return status;
  }

  return status;
}
