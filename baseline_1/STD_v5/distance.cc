// distance.cc


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

#include "distance.h"
namespace aslp_std {

void Smooth(const infra::matrix& dist, infra::matrix& sdist, int context) {
    unsigned long height = dist.height();
    unsigned long width = dist.width();

    int length = 2 * context + 1;

    for (int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            float sum = dist(i,j);
            for (int k=1; k <= context; k++) {
                int ii = std::max(0, i-k);
                int jj = std::max(0, j-k);
                sum+=dist(ii,jj);
                ii = std::min(int(height-1), i+k);
                jj = std::min(int(width-1), j+k);
                sum+=dist(ii, jj);
            }
            sdist(i, j) = sum / length;
        }
    }

}

void NormalizeFeature(infra::matrix& feature) {
    unsigned long height = feature.height();
    unsigned long width = feature.width();

    for(int i = 0; i < height; i++) {
        float sum=0;
        for(int j = 0; j < width; j++)
            sum += feature(i, j) * feature(i, j);

        float mod = sqrt(sum);
        for(int j = 0; j < width; j++)
            feature(i,j) /= mod;
    }

}

void MVN(infra::matrix& feature) {
    unsigned long height = feature.height();
    unsigned long width = feature.width();

    for(int i = 0; i < width; i++) {
        float mean = feature.column(i).sum() / height;
        feature.column(i) -= mean; // centralize
        float std  = sqrt((feature.column(i) * feature.column(i)) / height);
        feature.column(i) /= std;

    }

}

void KLDivergenceDistance(const infra::matrix& query, const infra::matrix& test, infra::matrix& dist_matrix) {
    unsigned long height = query.height();
    unsigned long width = test.height();
    unsigned long dim = query.width();
    infra::matrix matrix_query_log(height, dim);
    infra::matrix matrix_test_log(width, dim);
    infra::vector vector_query_log_query(height);
    infra::vector vector_test_log_test(width);
    infra::matrix matrix_query_log_test(height, width);
    infra::matrix matrix_test_log_query(width, height);
    infra::matrix matrix_test_log_query_T(height, width);
    infra::matrix matrix_eye(height, height);
    matrix_eye.eye();

    matrix_query_log.zeros();
    matrix_query_log = matrix_query_log + query;
    matrix_query_log.log();

    matrix_test_log.zeros();
    matrix_test_log = matrix_test_log + test;
    matrix_test_log.log();
    for (int i=0; i < height; i++) {
        vector_query_log_query(i) = query.row(i) * matrix_query_log.row(i);      
    }

    for (int i=0; i < width; i++) {
        vector_test_log_test(i) = test.row(i) * matrix_test_log.row(i);
    }

    infra::prod_t(query, matrix_test_log, matrix_query_log_test);
    infra::prod_t(test, matrix_query_log,matrix_test_log_query);

    matrix_query_log_test = 0 - matrix_query_log_test;
    matrix_test_log_query = 0 - matrix_test_log_query;
    infra::add_column_vector(matrix_query_log_test, vector_query_log_query);
    infra::add_column_vector(matrix_test_log_query, vector_test_log_test);
    infra::prod_t(matrix_eye,  matrix_test_log_query, matrix_test_log_query_T);  
    infra::sum(matrix_test_log_query_T, matrix_query_log_test, dist_matrix);
    dist_matrix = 0.5 * dist_matrix;
}

void CosineDistance(const infra::matrix& query, const infra::matrix& test, infra::matrix& dist_matrix) {
    // hear the mat_a and mat_b already done the variance normalization so the inner product equals to cosine distance
    infra::prod_t(query, test, dist_matrix);     // dist_matrix = query*test.T
    dist_matrix = 1 - dist_matrix;
}

void EuclideanDistance(const infra::matrix& query, const infra::matrix& test, infra::matrix& dist_matrix) {
    unsigned long height = query.height();
    unsigned long width = test.height();
    unsigned long dim = query.width();
    infra::vector diff(dim);
    for (int i=0; i < height; i++) {
        for (int j=0; j < width; j++) {
            diff = query.row(i)-test.row(j);
            dist_matrix(i, j)= sqrt(diff * diff);
        }
    }
}

void ComputeDist( const infra::matrix& query, const infra::matrix& test, infra::matrix& dist_matrix, std::string distance_type) {
    int height = query.height();
    int width = test.height();
    dist_matrix.resize(height, width);
    if (distance_type.find("KL-divergence") != std::string::npos) {
        KLDivergenceDistance(query, test, dist_matrix);
    } else if (distance_type.find("cos") != std::string::npos) {
        CosineDistance(query, test, dist_matrix);
    } else if (distance_type.find("euclideanDistance") != std::string::npos) {
        EuclideanDistance(query, test, dist_matrix);
    } else {
        std::cout << "We don't support this kind of distance type: " << distance_type << std::endl;
        exit(1);
    }
}

} //namespace aslp_std
