// template_average.cc

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

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib> 

#include "infra.h"
#include "htkfile.h"
#include "dataset.h"
#include "dtw.h"
#include "distance.h"
#include "feature.h"
#include "util.h"
#define MISPAR_GADOL_MEOD (1000000000);


using namespace std;

void TemplateAverageForInstances(const std::vector<infra::matrix> instances, infra::matrix &feature_avg) {
    int num = instances.size();
    int half = num/2;
    int i = 0;
    if (num == 1) {
        int height = instances[0].height();
        int dim = instances[0].width();
        feature_avg.resize(height, dim);
        feature_avg = instances[0];
    } else if (num == 2) {
        aslp_std::AverageTemplate(instances[0], instances[1], "cos", feature_avg);
    } else {
        infra::matrix avg_1(1, 1);
        infra::matrix avg_2(1, 1);
        std::vector<infra::matrix> subinstances_1;
        std::vector<infra::matrix> subinstances_2;
        for(i = 0; i < half; i++) {
            subinstances_1.push_back(instances[i]);
        }
        for(i = half; i < num; i++) { 
            subinstances_2.push_back(instances[i]);
        }

        TemplateAverageForInstances(subinstances_1, avg_1);
        TemplateAverageForInstances(subinstances_2, avg_2);
        aslp_std::AverageTemplate(avg_1, avg_2, "cos", feature_avg);
    }
    return;
}

void TemplateAverageForOneQuery(aslp_std::Feature *querys, const std::vector<int> &instance_ids, infra::matrix &feature_avg) {
    std::vector<infra::matrix> instances;
    for (int i=0; i< instance_ids.size(); i++) {
        instances.push_back(querys[ instance_ids[i] ].GetFeature());
    }  
    TemplateAverageForInstances(instances, feature_avg);
}

void TemplateAverage(aslp_std::Feature *querys, std::map< std::string, std::vector<int> > &uniq_query_map, 
                    std::string distance_type, std::string feature_type, std::string out_dir) {
    std::map<std::string, std::vector<int> >::iterator it;
    for (it=uniq_query_map.begin(); it != uniq_query_map.end(); it++) {
        infra::matrix feature_avg;
        TemplateAverageForOneQuery(querys, it->second, feature_avg);
        aslp_std::Feature feature_avg_object(out_dir, it->first, feature_type, feature_avg);
        std::cout << it->first << " " <<(it->second).size() << std::endl;
        feature_avg_object.WriteData(out_dir);
    }
}

void MVN(aslp_std::Feature* features, int feature_size) {
    for (int i=0; i < feature_size; i++) {
        features[i].DoMVN();
    }
}

void NormalizeFeature(aslp_std::Feature* features, int feature_size) {
    for (int i=0; i < feature_size; i++) {
        features[i].DoNormalizeFeature();
    }
}

void ReadData(aslp_std::Feature* features, string feature_dir, StringVector &feature_list, int feature_list_size, string feature_type) {
    for (int i=0; i < feature_list_size; i++) {
        features[i].ReadData(feature_dir, feature_list[i], feature_type); 
    }
}

int main(int argc, char *argv[]) {    
    if(argc < 7) {
        cerr<<"USAGE: query_dir query_list_file feature_type distance_type do_mvn result_dir" << endl;
        return EXIT_FAILURE;
    }
    
    std::string query_dir;
    std::string query_list_file;
    std::string feature_type;
    std::string distance_type;
    std::string out_dir;
    int do_mvn;

    query_dir = string(argv[1]);
    query_list_file = string(argv[2]);
    feature_type = string(argv[3]);
    distance_type = string(argv[4]);
    do_mvn = atoi(argv[5]);
    out_dir = string(argv[6]);

    StringVector query_list;
    query_list.read(query_list_file);

    
    int query_size = query_list.size();   
    // read test set
    aslp_std::Feature* querys = new aslp_std::Feature[query_size];
    
    ReadData(querys, query_dir, query_list, query_size, feature_type);
    if (do_mvn) {
        MVN(querys, query_size);
    }
    
    if (distance_type.find("cos") != std::string::npos ) { 
        NormalizeFeature(querys, query_size);
    }

    std::vector<std::string> query_id_splits;
    std::map< std::string, std::vector<int> > uniq_query_map;
    for (int i=0; i < query_size; i++) {
        std::string query_id = querys[i].GetFeatureId();
        query_id_splits = aslp_std::Split(query_id, "_");
        string keyword = query_id_splits[0];
        if (uniq_query_map.count(keyword) > 0) {
            uniq_query_map[keyword].push_back(i);
        } else {
            std::vector<int> instance_ids;
            uniq_query_map.insert(std::pair< std::string, std::vector<int> >(keyword, instance_ids) );
            uniq_query_map[keyword].push_back(i);
        }
    }
    TemplateAverage(querys, uniq_query_map, distance_type, feature_type, out_dir);    
    delete [] querys;
    return EXIT_SUCCESS;
}

