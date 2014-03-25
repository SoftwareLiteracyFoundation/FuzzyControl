#ifndef FCL_Keyword_H
#define FCL_Keyword_H

//---------------------------------------------------------------------
// struct FCL_keyword
//
// Purpose: Container for an FCL keyword, the keywords map in the
//          FuzzyControlClass is a collection of these, one for 
//          each keyword
//---------------------------------------------------------------------
struct FCL_keyword {
  string keyword;
  string meaning;
  int    index;
};

#endif
