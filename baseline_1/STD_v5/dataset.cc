
/**************************** INCLUDE FILES *****************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include "dataset.h"

/************************************************************************
 Function:     operator << for IntVector
 
 Description:  Write int vector to output stream
 Inputs:       std::ostream&, const IntVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const IntVector& v)
{
  IntVector::const_iterator iter = v.begin();
  IntVector::const_iterator end = v.end();
  
  while(iter < end) {
    os << *iter << " ";
    ++iter;
  }
  return os;
}

/************************************************************************
 Function:     operator << for StringVector
 
 Description:  Write std::string vector to output stream
 Inputs:       std::ostream&, const StringVector&
 Output:       std::ostream&
 Comments:     none.
 ***********************************************************************/
std::ostream& operator<< (std::ostream& os, const StringVector& v)
{
  StringVector::const_iterator iter = v.begin();
  StringVector::const_iterator end = v.end();
  
  while(iter < end) {
    os << *iter << " ";
    ++iter;
  }
  return os;
}

// --------------------------  EOF ------------------------------------//
