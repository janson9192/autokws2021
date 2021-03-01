// util.h

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
#ifndef QBE_STD_UTIL_H_
#define QBE_STD_UTIL_H_

#include <math.h>
#include <string> 
#include <vector>
namespace aslp_std { 

std::vector<std::string> Split(std::string str,std::string pattern);

}//namespace aslp_std

#endif
