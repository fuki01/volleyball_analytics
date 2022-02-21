#ifndef mainJclustring
#define mainJclustring


#include <stdio.h>
#include <string>
#include <iostream>
#include <cmath>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <functional>


class Jclustring {
public:
    Jclustring(std::string video_file_path, std::string shot_path, int N);
    
    void run();
private:
    int N = 0;
    std::vector<std::vector<int>> shot_list;
    std::vector<std::vector<int>> split_shot_list;
    std::vector<std::vector<std::vector<int>>> save_shot_list;
    std::vector<std::vector<std::vector<int>>> split_shot_hist_list;
    std::string video_file_path;
    std::string shot_path;
    cv::VideoCapture cap;
    
    std::vector<std::vector<int>> split_shot_list_root;
    std::vector<std::vector<std::vector<int>>> split_shot_hist_list_root;
    
    std::vector<std::vector<int>> comprison_tmp;
    
    std::vector<cv::Mat> frames;
    
    std::vector<double> en_list;
    
    double divide = 0.;
    
    void video_load();
    void debug();
    void read_shot_file();
    void split_shot();
    void create_key_frame_histgram();
    void save();
    void shot_comparison();
    int shot_distance(std::vector<std::vector<int>> shi, std::vector<std::vector<int>> shj);
    void end_process();
    
    std::vector<int> histogram_create(cv::Mat image);
    double histogram_distance(std::vector<int> shi, std::vector<int> shj);
    std::vector<int> histogram_marge(std::vector<int> hist, std::vector<int> hist_other, int Ni, int Nj);
    std::vector<double> histogram_avarage(std::vector<std::vector<int>> hist_vec);
    
    double euclidean_distance(std::vector<double> hist, std::vector<double> hist_avg);
    
    void recursive_comb(int *indexes, int s, int rest, std::function<void(int *)> f);
    void foreach_comb(int n, int k, std::function<void(int *)> f);
    
    void output();
    
};

#endif
