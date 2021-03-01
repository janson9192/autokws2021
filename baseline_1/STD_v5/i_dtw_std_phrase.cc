//=============================================================================
//
// File Name: Lab1.cpp
// Written by: Peng Yang
// Last Revise: Jingyong Hou
// Email: pengyang@nwpu-aslp.org
//
//=============================================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include "util.h"
#include "infra.h"
#include "htkfile.h"
#include "dataset.h"
#include "dtw.h"
#include "distance.h"
#include "feature.h"

#define MISPAR_GADOL_MEOD (1000000000);

using namespace std;
//using namespace aslp_std;
//this function is used to score for one query as the function name told
//parameter: 
//input : query_id //string
//          test_size //int
//          featureType //string
//output: test //the out put matrix
int score_for_one_query(aslp_std::Feature &query, aslp_std::Feature* tests, int test_size, 
                        string distance_type, string result_dir) {
    //normal DTW  
     
    std::string query_id = query.GetFeatureId();
    infra::matrix query_feature = query.GetFeature();
    ofstream ofs((result_dir + query_id + ".RESULT").c_str());
    //std::cout << "finish " << query_id << std::endl;
    // run over the test set                
    for (int i = 0; i < test_size; i++) {
    //    dynamic programming between query and test[i]
        infra::matrix test_feature = tests[i].GetFeature();
        unsigned long height = query_feature.height();
        unsigned long width = test_feature.height();
        infra::matrix dist(height, width);
        infra::vector area(2);    
        aslp_std::ComputeDist(query_feature, test_feature, dist, distance_type);
        // cout << "query id: " << query_id << ", length: " << height << endl;
        // cout << "utterance id: " << tests[i].GetFeatureId() << ", length: " << width <<endl;
  /*      int p,q;
        ofstream out;
        out.open(query_id+"_"+tests[i].GetFeatureId()+"_distance.txt", ios::out);

        for (p = 0; p < height; p++) {
            for (q = 0; q < width; q++) {
               out << dist(p,q)  << " ";
            }
            out << endl;
        }
        out.close();
    */    
        float score = aslp_std::I_DTW(0, dist, area);
        ofs << score << " " << area(0) << " " << area(1) << endl;
    }
    ofs.close();
    return EXIT_SUCCESS;
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

void ReadQueryData(aslp_std::Feature *features, int *features_break, string feature_dir, StringVector &feature_list, int feature_list_size, string feature_type) {
    for (int i=0; i < feature_list_size; i++) {
        std::vector <std::string> words = aslp_std::Split(feature_list[i], "-");
        aslp_std::Feature f1;
        aslp_std::Feature f2;
        f1.ReadData(feature_dir, words[0], feature_type);
        f2.ReadData(feature_dir, words[1], feature_type);
        infra::matrix mat1 = f1.GetFeature();
        infra::matrix mat2 = f2.GetFeature();
        int h1 = mat1.height();
        int h2 = mat2.height();
        infra::matrix mat(h1+h2, mat1.width());
        for (int j = 0; j < h1; j++) {
            mat.row(j) = mat1.row(j);
        }
        for (int j = h1; j < h1+h2; j++) {
            mat.row(j) = mat2.row(j-h1);
        }
        features[i].ReadData(feature_dir, feature_list[i], feature_type, mat);
        features_break[i] = h1;
    }
}

int main(int argc, char *argv[]) {    
    if(argc < 9) {
        cerr<<"USAGE: query_dir query_list_file test_dir test_list_file feature_type distance_type doMvn result_dir" << endl;
        return EXIT_FAILURE;
    }

    string query_dir;
    string query_list_file;
    string test_dir;
    string test_list_file;
    string feature_type;
    string distance_type;
    string result_dir;
    int do_mvn;

    query_dir = string(argv[1]);
    query_list_file = string(argv[2]);
    test_dir = string(argv[3]);
    test_list_file = string(argv[4]);
    feature_type = string(argv[5]);
    distance_type = string(argv[6]);
    do_mvn = atoi(argv[7]);
    result_dir = string(argv[8]);

    StringVector query_list;
    query_list.read(query_list_file);

    StringVector test_list;
    test_list.read(test_list_file);
    
    int test_size = test_list.size();
    int query_size = query_list.size();   
    // read test set
    aslp_std::Feature* tests = new aslp_std::Feature[test_size];
    aslp_std::Feature* querys = new aslp_std::Feature[query_size];
    int *querys_break = new int[query_size];
    
    ReadData(tests, test_dir, test_list, test_size, feature_type);
    ReadQueryData(querys, querys_break, query_dir, query_list, query_size, feature_type);

    NormalizeFeature(tests, test_size);
    NormalizeFeature(querys, query_size);

    //debug the function of score_for_one_query()
    for (int i = 0; i < query_size; i++){
        score_for_one_query(querys[i], tests, test_list.size(), distance_type, result_dir);
    }    
    
    delete [] tests;
    delete [] querys;
    delete [] querys_break;
    return EXIT_SUCCESS;
}

