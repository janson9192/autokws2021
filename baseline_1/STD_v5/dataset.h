
#ifndef _MY_DATASET_H_
#define _MY_DATASET_H_

/************************************************************************
 Project:  Phoeneme Classification
 Module:   Dataset Definitions
 Purpose:  Defines the data structs of instance and label
 Date:     21 Apr., 2005

 *************************** INCLUDE FILES ******************************/
#include <fstream>
#include <vector>
#include <map>
#include <utility>
#include <string.h>

#include "infra.h"

#define MAX_LINE_SIZE 4096

  
/***********************************************************************/
class IntVector : public std::vector<int> {
 public:
  IntVector() : std::vector<int>() {  }

  IntVector(uint size) : std::vector<int>(size) { }
  int read(std::string &filename) {
    std::ifstream ifs(filename.c_str());
    // check input file stream
    if (!ifs.good()) {
      std::cerr << "Error: unable to read integer list from " << filename << std::endl;
      exit(-1);
    }
    // delete the vector
    clear();
    // read size from the stream
    int value;
    int num_values;
    if (ifs.good()) 
      ifs >> num_values;
    while (ifs.good() && num_values--) {
      ifs >> value;
      push_back(value);
    }
    return size();
  }  
};

std::ostream& operator<< (std::ostream& os, const IntVector& v);


/***********************************************************************/
class StringVector : public std::vector<std::string> {
 public:

  int read(std::string &filename) {
    std::ifstream ifs;
    char line[MAX_LINE_SIZE];
    ifs.open(filename.c_str());
    if (!ifs.is_open()) {
      std::cerr << "Error: Uunable to read file list " << filename << std::endl;
      return 0;
    }    
    while (!ifs.eof()) {
      ifs.getline(line,MAX_LINE_SIZE);
      if (strcmp(line,""))
	    push_back(std::string(line));
    }
    return size();
  }
};

std::ostream& operator<< (std::ostream& os, const StringVector& v);




#endif // _MY_DATASET_H_
