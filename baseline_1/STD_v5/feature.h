// feature.h

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

#ifndef ASLP_STD_FEATURE_H_
#define ASLP_STD_FEATURE_H_

#include <iostream>

#include "infra.h"
#include "htkfile.h"
#include "dataset.h"
#include "distance.h"
#include "dtw.h"

namespace aslp_std {

class Feature {
public:
    Feature() {}

    Feature(std::string feature_dir, std::string feature_id, std::string feature_type);

    Feature(std::string feature_dir, std::string feature_id, std::string feature_type, infra::matrix &feature);
    
    Feature &operator = (const Feature &other);

    ~Feature();
public:
        
    void DoMVN();
   
    void ReadData(std::string feature_dir, std::string feature_id, std::string feature_type);

    void ReadData(std::string feature_dir, std::string feature_id, std::string feature_type, unsigned long vad_start, unsigned long vad_end);
    
    void ReadData(std::string feature_dir, std::string feature_id, std::string feature_type, infra::matrix &mat);

    void WriteData(std::string feature_dir);     

    void DoNormalizeFeature();

    infra::matrix &GetFeature();

    std::string GetFeatureId();
    
    std::string GetFeatureType();

    //void GetKLData();

    void Destroy();

private:
    std::string feature_id_;
    
    std::string feature_type_;
    
    infra::matrix feature_;
    
};

}// namespace 
#endif
