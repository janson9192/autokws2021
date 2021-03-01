// dtw.h

// Copyright 2017  ASLP (Author: jyhou@nwpu-aslp.org)

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

#include <math.h>
#include <string>
#include <vector>

#include "infra.h"

#ifndef ASLP_KWS_DTW_H_
#define ASLP_KWS_DTW_H_

namespace aslp_std {

#define LEN_PENALTY_DIAG 2
#define BIG_FLT 1e100
#define MAX_ITERATION 3
float DTW(const infra::matrix &dist);

float DTWWithPath(const infra::matrix &dist, infra::matrix &path);

float SLN_DTW(const infra::matrix &dist, infra::vector &area, infra::vector &frame_cost); 

float subsequence_DTW_c1(const infra::matrix &dist, infra::vector &area);

float subsequence_DTW_c2(const infra::matrix &dist, infra::vector &area);

float I_DTW(float epsilon, const infra::matrix &dist, infra::vector &area);

void Average_basis(const infra::matrix &mat_a, const infra::matrix &mat_b, int i, int j, int num, infra::matrix &avg_mat);

int Average(const infra::matrix &mat_a, const infra::matrix &mat_b, const infra::matrix &path, infra::matrix &avg_mat);

int AverageTemplate(const infra::matrix &mat_a, const infra::matrix &mat_b, const std::string distance_type, infra::matrix &avg_mat);

void Average4Maps(const std::vector<infra::matrix> &instances, const std::vector< std::vector < std::vector<int> > > &maps, infra::matrix &avg_mat);

void ConvertPath2Map(const infra::matrix &path, std::vector< std::vector<int> > &map);
} //namespace aslp_std

#endif // ASLP_KWS_DTW_H_
