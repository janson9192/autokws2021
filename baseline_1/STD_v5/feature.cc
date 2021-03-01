// feature.cc


//Copyright 2017  ASLP-NWPU(Author: jyhou@nwpu-aslp.org)
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
#include <stdlib.h>
#include <iostream>
#include "feature.h"

namespace aslp_std {

Feature::Feature(std::string feature_dir, std::string feature_id, std::string feature_type) {
    std::string test_filename(feature_dir + feature_id + "." + feature_type);
    this->feature_id_ = feature_id;
    this->feature_type_ = feature_type;
    if (!read_htk(test_filename, this->feature_)) {
        std::cerr << "Error: fail to read the feature in the: " << test_filename << std::endl;
        exit(1);
    }
}

Feature::Feature(std::string feature_dir, std::string feature_id, std::string feature_type, infra::matrix &feature) {
    this->feature_id_ = feature_id;
    this->feature_type_ = feature_type;
    int height = feature.height();
    int dim = feature.width();
    this->feature_.resize(height, dim);
    this->feature_ = feature;
}

Feature::~Feature() {
    Destroy();
}
    
Feature &Feature::operator = (const Feature &other) {
    Destroy();
    this->feature_id_ = other.feature_id_;
    this->feature_type_ = other.feature_type_;
    this->feature_ = other.feature_;
}

void Feature::ReadData(std::string feature_dir, std::string feature_id, std::string feature_type) {
    std::string test_filename(feature_dir + feature_id + "." + feature_type);
    this->feature_id_ = feature_id;
    this->feature_type_ = feature_type;
    if (!read_htk(test_filename, this->feature_)) {
        std::cerr << "Error: fail to read the feature in the: " << test_filename << std::endl;
        exit(1);
    }
}

void Feature::ReadData(std::string feature_dir, std::string feature_id, std::string feature_type, unsigned long vad_start, unsigned long vad_end) {
    std::string test_filename(feature_dir + feature_id + "." + feature_type);
    this->feature_id_ = feature_id;
    this->feature_type_ = feature_type;

    infra::matrix feature_tmp;
    if (!read_htk(test_filename, feature_tmp)) {
        std::cerr << "Error: fail to read the feature in the: " << test_filename << std::endl;
        exit(1);
    }

    unsigned long mat_width, mat_height;
    mat_height = vad_end - vad_start;
    mat_width = feature_tmp.width();
    this->feature_.resize(mat_height, mat_width);
    this->feature_ = feature_tmp.submatrix(vad_start, 0, mat_height, mat_width);
}

void Feature::ReadData(std::string feature_dir, std::string feature_id, std::string feature_type, infra::matrix &feature) {
    this->feature_id_ = feature_id;
    this->feature_type_ = feature_type;
    int height = feature.height();
    int dim = feature.width();
    this->feature_.resize(height, dim);
    this->feature_ = feature;
}

void Feature::WriteData(std::string feature_dir) {
    std::string test_filename(feature_dir + this->feature_id_ + "." + this->feature_type_);
    if (!write_htk(test_filename, this->feature_)) {
        std::cerr << "Error: fail to read the feature in the: " << test_filename << std::endl;
        exit(1);
    }
}

void Feature::DoMVN() {
    MVN(this->feature_);
}

void Feature::DoNormalizeFeature() {
    NormalizeFeature(this->feature_);
}

infra::matrix &Feature::GetFeature() {
    return this->feature_;
}

std::string Feature::GetFeatureId() {
    return this->feature_id_;
}

std::string Feature::GetFeatureType() {
    return this->feature_type_;
}

void Feature::Destroy() {
    return;
}

}//namespace aslp_std
