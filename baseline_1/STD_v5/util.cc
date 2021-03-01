// util.cc

// Copyright 2017  ASLP (Author: jyhou@nwpu-aslp.org)
//
// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.
#include "util.h"

namespace aslp_std {

std::vector<std::string> Split(std::string str,std::string pattern) {
  std::string::size_type pos;
  std::vector<std::string> result;
  str+=pattern;
  int size=str.size();

  for(int i=0; i<size; i++) {
      pos=str.find(pattern,i);
      if(pos<size) {
          std::string s=str.substr(i,pos-i);
          result.push_back(s);
          i=pos+pattern.size()-1;
      }
  }
  return result;
}

}//namespace aslp_std

