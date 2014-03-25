#ifndef Fuzzy_Control_H
#define Fuzzy_Control_H

using namespace std;
#include <iostream>  // cout, cerr, ....
#include <fstream>   // to open the FCL file via ifstream object
#include <vector>    // vector of strings to hold the FCL lines
#include <string>    // string methods
#include <map>       // FCL keywords, InputVariables, OutputVariables, Rules
#include <cmath>     // fabs
#include <cstdlib>   // atof
#include <algorithm> // find

#include "FCLL_Version.h"
#include "FCL_Keyword.h"
#include "FuzzyInput.h"
#include "FuzzyOutput.h"
#include "FuzzyRules.h"

//#define DEBUG
//#define DEBUG_ALL

// Consider doubles equal to zero if less than this value
#define ZERO_TOLERANCE 1.E-6

// Definition of membership function types
enum term { Trapezoid,
	    Triangle,
	    Ramp,
	    Rectangle,
	    Singleton };

// Define direction of search for FindNoWhiteSpace()
enum direction { back, forward };

//------------------------------------------------------------------
// class FuzzyControl
//
// Purpose: Parent class/container for fuzzy controller using FCL
//------------------------------------------------------------------
class FuzzyControlClass {

 protected:
  int status; // Error code variable

  string inputFCL; // string to hold single line of FCL during read
  vector< string > FCLFileVector; // vector to hold all FCL file lines

  ifstream FCLFileStream;    // FCL file stream access object
  ofstream OutputDataStream; // Output data file stream access object

  string FCLFileName;      // FCL file name container
  string outputFileName;   // output data file name 
  string inputDelimeters;  // Line parsing delimeters for input data file
  string inputDataLabel;   // Column name to put labels in InputLabels

  // The FCL keyword map, the key is a string which is the keyword, 
  // the values are pointer to FCL_keyword struct, one for each keyword
  // See FCL_Keyword.h for definition of FCL_keyword struct.
  map < string, FCL_keyword* > keywords; 

  // FuzzyInputClass Map: The input variable map.
  // The key is a string which is the name of the input variable, 
  // the values are a FuzzyInputClass, one for each variable.
  // Each FuzzyInputClass (input variable) contains a map of input
  // fuzzification terms (membership functions): InputTerms. The 
  // InputTerms map has one entry for each fuzzification term.
  map < string, FuzzyInputClass* > InputVariables; 

  // FuzzyOutputClass Map: The output variable map.
  // The key is a string which is the name of the output variable, 
  // the values are a FuzzyOutputClass, one for each variable
  // Each FuzzyOutputClass (output variable) contains a map of output
  // defuzzification terms (membership functions): OutputTerms. The 
  // OutputTerms map has one entry for each defuzzification term.
  map < string, FuzzyOutputClass* > OutputVariables; 

  // FuzzyRulesClass Map: The rules map.
  // The key is a string which is the name of the rule, 
  // the values are a FuzzyRuleClass, one for each rule.
  map < string, FuzzyRuleClass* > Rules; 

  // Input Data container, string keyword is input variable name
  // vector is stack of data values (timeseries) for that variable
  map < string, vector< double >* > InputData; 

  // If there is a inputDataLabel input, this stack holds the labels
  vector < string > InputLabels;

 public:
  // Encapsulation methods for protected variables
  string  FCLFile()  const { return FCLFileName; }
  string &FCLFile()        { return FCLFileName; }

  string  OutputFileName() const { return outputFileName; }
  string &OutputFileName()       { return outputFileName; }

  string  InputDelimeters() const { return inputDelimeters; }
  string &InputDelimeters()       { return inputDelimeters; }  

  string  InputDataLabel()  const { return inputDataLabel; }
  string &InputDataLabel()        { return inputDataLabel; }

  map<string, FCL_keyword*>  Keywords()  const { return keywords; }
  map<string, FCL_keyword*> &Keywords()        { return keywords; }
  
  map <string, FuzzyInputClass*> InputVariablesMap() 
    const { return InputVariables; }
  map <string, FuzzyInputClass*> &InputVariablesMap() 
    { return InputVariables; }

  map <string, FuzzyOutputClass*>  OutputVariablesMap() 
    const { return OutputVariables; }
  map <string, FuzzyOutputClass*> &OutputVariablesMap() 
    { return OutputVariables; }

  map <string, FuzzyRuleClass*>  RulesMap() const { return Rules; }
  map <string, FuzzyRuleClass*> &RulesMap()       { return Rules; }

  map <string, vector<double>* > InputDataMap() const { return InputData; }
  map <string, vector<double>* > InputDataMap()       { return InputData; }

  // Access pointers into the keywords map for convenience
  // These are publically accessible, and probably shouldn't be,
  // but they are not advertised.
  FCL_keyword* keyword_Accu;         // pointer for ACCU
  FCL_keyword* keyword_ACT;          // pointer for ACT
  FCL_keyword* keyword_And;          // pointer for AND
  FCL_keyword* keyword_Asum;         // pointer for ASUM
  FCL_keyword* keyword_Bdif;         // pointer for BDIF
  FCL_keyword* keyword_Bsum;         // pointer for BSUM
  FCL_keyword* keyword_COA;          // pointer for COA
  FCL_keyword* keyword_COG;          // pointer for COG
  FCL_keyword* keyword_COGS;         // pointer for COGS
  FCL_keyword* keyword_Default;      // pointer for DEFAULT
  FCL_keyword* keyword_Defuzzify;    // pointer for DEFUZZIFY
  FCL_keyword* keyword_EndDefuzzify; // pointer for END_DEFUZZIFY
  FCL_keyword* keyword_EndFuncBlock; // pointer for END_FUNCTION_BLOCK
  FCL_keyword* keyword_EndFuzzify;   // pointer for END_FUZZIFY
  FCL_keyword* keyword_EndOpt;       // pointer for END_OPTIONS
  FCL_keyword* keyword_EndRule;      // pointer for END_RULEBLOCK
  FCL_keyword* keyword_EndVar;       // pointer for END_VAR
  FCL_keyword* keyword_FuncBlock;    // pointer for FUNCTION_BLOCK
  FCL_keyword* keyword_Fuzzify;      // pointer for FUZZIFY
  FCL_keyword* keyword_IF;           // pointer for IF
  FCL_keyword* keyword_IS;           // pointer for IS
  FCL_keyword* keyword_LM;           // pointer for LM
  FCL_keyword* keyword_Max;          // pointer for MAX
  FCL_keyword* keyword_Method;       // pointer for METHOD
  FCL_keyword* keyword_Min;          // pointer for MIN
  FCL_keyword* keyword_NC;           // pointer for NC
  FCL_keyword* keyword_Not;          // pointer for NOT
  FCL_keyword* keyword_Nsum;         // pointer for NSUM
  FCL_keyword* keyword_Options;      // pointer for OPTIONS
  FCL_keyword* keyword_Or;           // pointer for OR
  FCL_keyword* keyword_Prod;         // pointer for PROD
  FCL_keyword* keyword_Range;        // pointer for RANGE
  FCL_keyword* keyword_RM;           // pointer for RM
  FCL_keyword* keyword_Rule;         // pointer for RULE
  FCL_keyword* keyword_RuleBlock;    // pointer for RULEBLOCK
  FCL_keyword* keyword_Term;         // pointer for TERM
  FCL_keyword* keyword_Then;         // pointer for THEN
  FCL_keyword* keyword_Var;          // pointer for VAR
  FCL_keyword* keyword_VarIn;        // pointer for VAR_INPUT
  FCL_keyword* keyword_VarOut;       // pointer for VAR_OUTPUT
  FCL_keyword* keyword_With;         // pointer for WITH

  // FuzzyControl Methods
  // Constructor
  FuzzyControlClass( string FCLFileName, 
		     string inputDelimeters,
		     string inputDataLabel ); 

  int Fuzzification( int inputDataIndex );
  int FuzzifyInput ( string varName, double inputValue );

  int Aggregation();

  int Activation();

  int Accumulation();
  int Accumulate( FuzzyOutputClass* lpFOC, 
		  FuzzyOutputTerm* lpFACT, FuzzyOutputTerm* lpFACCU );

  int Defuzzification();

  // FCL File IO Methods
  int ReadFCLFile         ();
  int ReadInputDataFile   ( string *fileName );
  int OpenOutputFile      ( string *fileName );
  int CloseOutputFile     ();
  int WriteTimestepOutput ( int iteration );
  int WriteOutputHeader   ();
  int WriteOutput         ();

  // FCL Parsing Methods
  int ParseFCLFile             ();
  int LoadFCLKeywords          ( map< string, FCL_keyword* >& keywords );
  int ParseFCL_CheckFile       ();
  int ParseFCL_IO_Vars         ( FCL_keyword* kwdStart, FCL_keyword* kwdEnd );
  int ParseFCL_Input_Fuzzify   ();
  int ParseFCL_Output_Defuzzify();
  int ParseFCL_Rules           ();

  int SplitLine( vector<string> *subCondTerms, 
		 string *conditionString, string *delimeters );

  string::size_type FindNoWhiteSpace( string str, 
				      string::size_type offset, int direction );

  FCL_keyword* FindFCLKeywordFromMap( string key, bool err );

  bool CommentLine( string *line );

  int GetLine( vector< string >::iterator FCLFileIterator, string* line );

  int CheckFuzzyTermParen( string * FCLFileLine );
};

// API functions in FuzzyControlAPI.cc
int FuzzyControl_SingleInput ( FuzzyControlClass* lpFC );
int FuzzyControl_SeriesInput ( FuzzyControlClass* lpFC, int numDataPoints );
int FuzzyControl_Fuzzify     ( FuzzyControlClass* lpFC, 
			       string varName, double inputValue );
int FuzzyControl_Aggregation     ( FuzzyControlClass* lpFC );
int FuzzyControl_Activation      ( FuzzyControlClass* lpFC );
int FuzzyControl_Accumulation    ( FuzzyControlClass* lpFC );
int FuzzyControl_Defuzzification ( FuzzyControlClass* lpFC );

int FuzzyControl_ReadFCL          ( FuzzyControlClass* lpFC );
int FuzzyControl_IO_Files         ( FuzzyControlClass* lpFC, 
				    string* inFile, string* outFile,
				    int* numPointsRead );
int FuzzyControl_ReadDataFile     ( FuzzyControlClass* lpFC, 
				    string *inFile, int *numPointsRead );
int FuzzyControl_OpenOutputFile   ( FuzzyControlClass* lpFC, string* outFile );
int FuzzyControl_WriteOutputHeader( FuzzyControlClass* lpFC );
int FuzzyControl_WriteOutputData  ( FuzzyControlClass* lpFC );
int FuzzyControl_CloseOutputFile  ( FuzzyControlClass* lpFC );

int FuzzyControl_PrintKeywordMap        ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintInputVariableMap  ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintOutputVariableMap ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintRuleMap           ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintFuzzifiedInput    ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintAggregation       ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintActivation        ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintAccumulation      ( FuzzyControlClass* lpFC );
int FuzzyControl_PrintDefuzzifiedOutput ( FuzzyControlClass* lpFC );

// General console I/O functions in ConsoleMsg.cc, FCL_AccessoryFunc.cc
void ErrMsg     ( const char *msg, const char *arg, int status );
void ErrMsg     ( const char *msg, string arg,      int status );
void ErrMsg     ( const char *msg, int arg,         int status );
void ErrMsg     ( const char *msg, double arg,      int status );
void ErrMsg     ( string      msg, const char *arg, int status );
void ErrMsg     ( string      msg, string arg,      int status );
void ErrMsg     ( string      msg, int arg,         int status );
void ErrMsg     ( string      msg, double arg,      int status );
void ConsoleMsg ( const char *msg, const char *arg, int status );
void ConsoleMsg ( const char *msg, string arg,      int status );
void ConsoleMsg ( const char *msg, int arg,         int status );
void ConsoleMsg ( const char *msg, double arg,      int status );
void ConsoleMsg ( string      msg, const char *arg, int status );
void ConsoleMsg ( string      msg, string arg,      int status );
void ConsoleMsg ( string      msg, int arg,         int status );
void ConsoleMsg ( string      msg, double arg,      int status );

#ifdef DEBUG
void DebugMsg   ( const char *msg, const char *arg, int status );
void DebugMsg   ( const char *msg, string arg,      int status );
void DebugMsg   ( const char *msg, int arg,         int status );
void DebugMsg   ( const char *msg, double arg,      int status );
void DebugMsg   ( string      msg, const char *arg, int status );
void DebugMsg   ( string      msg, string arg,      int status );
void DebugMsg   ( string      msg, int arg,         int status );
void DebugMsg   ( string      msg, double arg,      int status );
#else
#define DebugMsg
#endif
#ifdef DEBUG_ALL
void DebugAllMsg( const char *msg, const char *arg, int status );
void DebugAllMsg( const char *msg, string arg,      int status );
void DebugAllMsg( const char *msg, int arg,         int status );
void DebugAllMsg( const char *msg, double arg,      int status );
void DebugAllMsg( string      msg, const char *arg, int status );
void DebugAllMsg( string      msg, string arg,      int status );
void DebugAllMsg( string      msg, int arg,         int status );
void DebugAllMsg( string      msg, double arg,      int status );
#else
#define DebugAllMsg
#endif

void PrintKeywordMap       ( map< string, FCL_keyword* >      *keywords );
void PrintRuleMap          ( map< string, FuzzyRuleClass* >   *rules );
void PrintInputVariableMap ( map< string, FuzzyInputClass* >  *fic );
void PrintOutputVariableMap( map< string, FuzzyOutputClass* > *foc );
void PrintFuzzifiedInput   ( map< string, FuzzyInputClass* >  *fic );
void PrintDefuzzifiedOutput( map< string, FuzzyOutputClass* > *foc );

#endif
