// dtw.cc

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

#include "dtw.h"
#include "distance.h"
namespace aslp_std {
    
float DTW(const infra::matrix &dist) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();
    infra::matrix cost(height+1, width+1);
    infra::matrix length(height+1, width+1);
    cost(0,0) = 0.0;
    length(0,0) = 1;
    for (int i=1; i < height+1; i++) {
        cost(i, 0) = BIG_FLT;
        length(i, 0) = 1;
    }
    for (int j=1; j < width+1; j++) {
        length(0, j) = 1;
        cost(0, j) = BIG_FLT;
    }
    for (int i=1; i < height+1; i++) {
        for (int j=1; j < width+1; j++) {
            if (cost(i-1, j) < cost(i, j-1) && cost(i-1, j) < cost(i-1, j-1)){
                length(i, j) = length(i-1, j) + 1;
            } else if (cost(i-1, j-1) <= cost(i, j-1) && cost(i-1, j-1) <= cost(i-1, j)) {
                length(i, j) = length(i-1, j-1) + 1;
            } else if (cost(i, j-1) <= cost(i-1, j) && cost(i, j-1) <= cost(i-1, j-1)) {
                length(i, j) = length(i, j-1) + 1;
            } else {
                std::cout << "WARNNING: we do not find the min value of DTW distance" << std::endl;
            }
            cost(i, j) = std::min(std::min(cost(i-1, j), cost(i, j-1)), cost(i-1, j-1)) + dist(i-1, j-1);
        }
    }
    return cost(height, width) / length(height, width);
}

float DTWWithPath(const infra::matrix &dist, infra::matrix &path) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();
    infra::matrix cost(height+1, width+1);
    infra::matrix length(height+1, width+1);
    path.resize(height+1, width+1);

    cost(0,0) = 0.0;
    length(0,0) = 1;
    path(0, 0) = -1;
    for (int i=1; i < height+1; i++) {
        cost(i, 0) = BIG_FLT;
        length(i, 0) = 1;
        path(i, 0) = -1;
    }
    for (int j=1; j < width+1; j++) {
        cost(0, j) = BIG_FLT;
        length(0, j) = 1;
        path(0, j) = -1;
    }
    for (int i=1; i < height+1; i++) {
        for (int j=1; j < width+1; j++) {
            if (cost(i-1, j) <= cost(i, j-1) && cost(i-1, j) <= cost(i-1, j-1)){
                length(i, j) = length(i-1, j) + 1;
                path(i, j) = 1;
            } else if (cost(i-1, j-1) <= cost(i, j-1) && cost(i-1, j-1) <= cost(i-1, j)) {
                length(i, j) = length(i-1, j-1) + 1;
                path(i, j) = 2;
            } else if (cost(i, j-1) <= cost(i-1, j) && cost(i, j-1) <= cost(i-1, j-1)) {
                length(i, j) = length(i, j-1) + 1;
                path(i, j) = 3;
            } else {
                std::cout << "WARNNING: we do not find the min value of DTW distance" << std::endl;
            }
            cost(i, j) = std::min(std::min(cost(i-1, j), cost(i, j-1)), cost(i-1, j-1)) + dist(i-1, j-1);
        }
    }
    return cost(height, width)/length(height, width);
}

//this dtw use local constrain: 
// (i-1,j)
// (i,j-1)
// (i-1,j-1)
float SLN_DTW(const infra::matrix &dist, infra::vector &area, infra::vector &frame_cost) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();
    infra::matrix avg_cost(height, width);
    infra::matrix cost(height, width);
    infra::matrix trace(height, width);
    infra::matrix length(height, width);

    // initialize
    // trace(i,j) = 0 denote the precedent point of (i,j) is (i-1,j)
    // trace(i,j) = 1                                        (i,j-1)
    // trace(i,j) = 2                                        (i-1,j-1)
    for( int i = 0; i < width; i++) {
        avg_cost(0, i) = dist(0, i);
        cost(0, i) = dist(0, i);
        trace(0, i) = i;
        length(0, i) = 1;
    }

    for( int i = 1; i < height; i++) {
        trace(i, 0) = 0;
        length(i, 0) = i + 1;
        cost(i, 0) = dist(i, 0) + cost(i-1, 0);
        avg_cost(i, 0)= cost(i, 0) / length(i, 0);
    }

    // fill the three matrices in a dynamic programming style.
    for (int i = 1; i < height; i++) {
        for (int j = 1; j < width; j++) {
            // compute the three possible costs
            double cost_0 = dist(i, j) + cost(i-1, j);
            double cost_1 = dist(i, j) + cost(i, j-1);
            double cost_2 = dist(i, j) + cost(i-1, j-1);
            double avg_cost_0 = cost_0 / (1 + length(i-1, j));
            double avg_cost_1 = cost_1 / (1 + length(i, j-1));
            double avg_cost_2 = cost_2 / (LEN_PENALTY_DIAG + length(i-1, j-1));

            // choose the one which lead to the minimum cost as the precedent point
            if(avg_cost_0 < avg_cost_1) {
                if(avg_cost_0 < avg_cost_2) {
                    avg_cost(i, j) = avg_cost_0;
                    cost(i, j) = cost_0;
                    length(i, j) = 1 + length(i-1, j);
                    trace(i, j) = trace(i-1, j);
                }
                else {
                    avg_cost(i, j) = avg_cost_2;
                    cost(i, j) = cost_2;
                    length(i, j) = LEN_PENALTY_DIAG + length(i-1, j-1);
                    trace(i, j) = trace(i-1, j-1);
                }
            } else if(avg_cost_1 < avg_cost_2) {
                avg_cost(i, j) = avg_cost_1;
                cost(i, j) = cost_1;
                length(i, j) = 1 + length(i, j-1);
                trace(i, j) = trace(i, j-1);
            } else {
                avg_cost(i, j) = avg_cost_2;
                cost(i, j) = cost_2;
                length(i, j) = LEN_PENALTY_DIAG + length(i-1, j-1);
                trace(i, j) = trace(i-1, j-1);
            }

        }
    }
    
    int end_point = avg_cost.row(height-1).argmin();
    float min_cost = avg_cost(height-1, end_point);
    int start_point = trace(height-1, end_point);
    area(0) = start_point;
    area(1) = end_point;
    frame_cost.resize(width);
    frame_cost = avg_cost.row(height-1);
    return min_cost;
}

float subsequence_DTW_c1(const infra::matrix &dist, infra::vector &area) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();
    infra::matrix cost(height, width);
    infra::matrix length(height, width);
    infra::matrix path(height, width);
    infra::matrix s_point(height, width);

    cost(0, 0) = dist(0, 0);
    length(0, 0) = 1;
    path(0, 0) = -1;
    s_point(0, 0) = 0;
    int i, j; 
    //first colum
    for (i=1; i < height; i++) {
        cost(i, 0) = cost(i-1, 0) + dist(i, 0);
        length(i, 0) = length(i-1, 0) + 1;
        path(i, 0) = 3;
        s_point(i, 0) = s_point(i-1, 0);
    }
    //first row
    for (j=1; j < width; j++) {
        cost(0, j) = cost(0, j-1) + dist(0, j);
        length(0, j) = length(0, j-1) + 1;
        path(0, j) = 1;
        s_point(0, j) = s_point(0, j-1) + 1;
    }

    for (i = 1; i < height; i++) {
        for (j = 1; j < width; j++) {
            if (cost(i-1, j) <= cost(i, j-1) && cost(i-1, j) <= cost(i-1, j-1)){
                length(i, j) = length(i-1, j) + 1;
                path(i, j) = 3;
                s_point(i, j) = s_point(i-1, j);
                cost(i, j) = cost(i-1, j) + dist(i, j);
            } else if (cost(i-1, j-1) <= cost(i, j-1) && cost(i-1, j-1) <= cost(i-1, j)) {
                length(i, j) = length(i-1, j-1) + 1;
                path(i, j) = 2;
                s_point(i, j) = s_point(i-1, j-1);
                cost(i, j) = cost(i-1, j-1) + dist(i, j);
            } else if (cost(i, j-1) <= cost(i-1, j) && cost(i, j-1) <= cost(i-1, j-1)) {
                length(i, j) = length(i, j-1) + 1;
                path(i, j) = 1;
                s_point(i, j) = s_point(i, j-1);
                cost(i, j) = cost(i, j-1) + dist(i, j);
            } else {
                std::cout << "WARNNING: we do not find the min value of DTW distance" << std::endl;
            }
        }
    }
    // here we get the spotting area and calculate the epsilon
    i = height-1;
    j = width-1;
    while(path(i, j) > 0.5  &&  path(i, j) < 1.5) {
        j--;
    }

    int s = int(s_point(i, j));
    area(0) = s;
    area(1) = j;
    return (cost(i, j)-cost(0, s)-dist(i, j))/(length(i, j)-length(0, s)-1);
}

float subsequence_DTW_c2(const infra::matrix &dist, infra::vector &area) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();
    infra::matrix cost(height, width);
    infra::matrix length(height, width);
    infra::matrix path(height, width);
    infra::matrix s_point(height, width);

    cost(0, 0) = dist(0, 0);
    length(0, 0) = 1;
    path(0, 0) = -1;
    s_point(0, 0) = 0;
    int i, j; 
    double cost_0, cost_1, cost_2;
    //first colum
    for (i=1; i < height; i++) {
        cost(i, 0) = BIG_FLT;
        length(i, 0) = 1;
        path(i, 0) = -1;
        s_point(i, 0) = j;
    }
    //first row
    for (j=1; j < width; j++) {
        cost(0, j) = cost(0, j-1) + dist(0, j);
        length(0, j) = j+1;
        path(0, j) = 1;
        s_point(0, j) = j;
    }

    for (i = 1; i < height; i++) {
        for (j = 1; j < width; j++) {
            if (i < 2) {
                cost_0 = BIG_FLT*2;
            } else {
                cost_0 = cost(i-2, j-1);
            }
            cost_1 = cost(i-1, j-1);
            cost_2 = cost(i, j-1);
            if (cost_0 <= cost_1 && cost_0 <= cost_2){
                length(i, j) = j + 1; 
                path(i, j) = 3;
                s_point(i, j) = s_point(i-2, j-1);
                cost(i, j) = cost_0 + dist(i, j);
            } else if (cost_1 <= cost_0 && cost_1 <= cost_2) {
                length(i, j) = j + 1;
                path(i, j) = 2;
                s_point(i, j) = s_point(i-1, j-1);
                cost(i, j) = cost_1 + dist(i, j);
            } else if (cost_2 <= cost_0 && cost_2 <= cost_1) {
                length(i, j) = j + 1;
                path(i, j) = 1;
                s_point(i, j) = s_point(i, j-1);
                cost(i, j) = cost_2 + dist(i, j);
            } else {
                std::cout << "WARNNING: we do not find the min value of DTW distance" << std::endl;
            }
        }
    }
    // here we get the spotting area and calculate the epsilon
    i = height-1;
    j = width-1;
    while(path(i, j) > 0.5  &&  path(i, j) < 1.5) {
        j--;
    }
    int s = int(s_point(i, j));
    area(0) = s+1;
    area(1) = j-1;
    return (cost(i, j)-cost(0, s)-dist(i, j))/(length(i, j)-length(0, s)-1);
}

float I_DTW(float epsilon, const infra::matrix &dist, infra::vector &area) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();

    int i,j;
    int counter=0;
    float best_epsilon = 1;

    //make new distance matrix for subsequence dtw
    infra::matrix dist_tmp(height+2, width);
    for (i = 1; i < height+1; i++) {
        dist_tmp.row(i) = dist.row(i-1);
    }

    while ((best_epsilon - epsilon) > 1e-5f || best_epsilon < epsilon) {
        best_epsilon = epsilon;
        for(j = 0; j < width; j++) {
            dist_tmp(0, j) = epsilon;
            dist_tmp(height + 1, j) = epsilon;
        }
        epsilon = subsequence_DTW_c2(dist_tmp, area);
        // std::cout << "iteration: " << counter << ", epsilon: " << epsilon << ", start frame: " << area(0) << ", end frame: " << area(1) << std::endl;
        if (epsilon > 1) {
            std::cout << "too short utterances: (" << width  << " vs " 
                      << height << ")" << std::endl;
            break;
        }
        counter++;
        if (counter >= MAX_ITERATION) {
            //std::cout << "exceed maximum iterations: " << counter << std::endl;
            //std::cout << "precision: " << best_epsilon - epsilon << std::endl;
            break;
        }
    }
    //std::cout << std::endl;
    return epsilon;
}

void Average_basis(const infra::matrix &mat_a, const infra::matrix &mat_b, int i, int j, int num, infra::matrix &avg_mat) {
    unsigned long dim = mat_a.width(); 
    infra::vector vec_a(dim);
    vec_a.zeros();
    vec_a = mat_a.row(i-1);
    if (num == 1) {
        vec_a += mat_b.row(j-1);
        vec_a *= 0.5;
    } else {
        for (int index =0; index < num; index++) {
            vec_a += mat_b.row(j-index-1);
        }
        vec_a /= (num+1);
    }
    for (int index = 0; index < dim; index++) {
        avg_mat(i-1, index) = vec_a(index);
    }
}


int Average(const infra::matrix &mat_a, const infra::matrix &mat_b, const infra::matrix &path, infra::matrix &avg_mat) { 
    int i = mat_a.height();
    int j = mat_b.height();
    avg_mat.resize(i, mat_a.width());
    while(i > 0) {
        if (path(i,j) < 0.5) {
            std::cout << "ERROR: no such path 1 " << std::endl;
        } else if (path(i, j) < 1.5 && path(i, j) > 0.5) {
            Average_basis(mat_a, mat_b, i, j, 1, avg_mat);
            i--;
        } else if (path(i, j) < 2.5 && path(i, j) > 1.5) {
            Average_basis(mat_a, mat_b, i, j, 1, avg_mat);
            i--;
            j--;
        } else if (path(i, j) < 3.5 && path(i, j) > 2.5) {
            int k = j;
            while (path(i, j) < 3.5 && path(i, j) > 2.5) {
                j--;
            }
            Average_basis(mat_a, mat_b, i, k, k-j, avg_mat);
            i--;
            j--;
        } else {
            std::cout << "ERROR: no such path " << std::endl;
        }
    }
    return i;
}

int AverageTemplate(const infra::matrix &mat_a, const infra::matrix &mat_b, std::string distance_type, infra::matrix &avg_mat) {
    unsigned long height = mat_a.height();
    unsigned long width = mat_b.height();
    infra::matrix path(height + 1, width + 1);
    infra::matrix dist(height, width);  
    
    ComputeDist(mat_a, mat_b, dist, distance_type);
    DTWWithPath(dist, path);
    avg_mat.resize(height, mat_a.width());
    Average(mat_a, mat_b, path, avg_mat);
}

void Average4Maps(const std::vector<infra::matrix> &instances, const std::vector< std::vector <  std::vector<int> > > &maps, infra::matrix &avg_mat) {
    int i, j, k;
    int length = maps[0].size();
    int dim = instances[0].width();
    avg_mat.resize(length, dim);
    avg_mat.zeros();
    float count =0;
    for (i = 0; i < length; i++) {
        count =0;
        for (j = 0; j < instances.size(); j++) {
            for (k = 0; k < maps[j][i].size(); k++) {
                avg_mat.row(i) += instances[j].row(maps[j][i][k]);
                count += 1;
            }
        }
        avg_mat.row(i) /= count;
    }
}

void ConvertPath2Map(const infra::matrix &path, std::vector< std::vector<int> > &map
) {
    int i = path.height()-1;
    int j = path.width()-1;
    std::vector<int> *p = new std::vector<int>;
    for (int k = 0; k < i; k ++) {
        p = new std::vector<int>;
        map.push_back(*p);
    }
    while( i > 0) {
        if (path(i,j) < 0.5) {
            std::cout << "ERROR: no such path " << std::endl;
        } else if (path(i, j) < 1.5 && path(i, j) > 0.5) {
            map[i-1].push_back(j-1);
            i--;
        } else if (path(i, j) < 2.5 && path(i, j) > 1.5) {
            map[i-1].push_back(j-1);
            i--;
            j--;
        } else if (path(i, j) < 3.5 && path(i, j) > 2.5) {
            int k = j;
            while (path(i, j) < 3.5 && path(i, j) > 2.5) {
                map[i-1].push_back(j-1);
                j--;
            }
            map[i-1].push_back(j-1);
            i--;
            j--;
        } else {
            std::cout << "ERROR: no such path " << std::endl;
        }
    }
}
} //namespace aslp_std
