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
    bool set_video();
    cv::VideoCapture cap;
//    分割キーフレーム数
    int N;
//    ショット位置が書かれたファイルパス
    std::string shot_path;
//    ビデオのファイルパス
    std::string video_file_path;
    
//    1.Segment the sports video into shots, S={s1 s2 ,sN }, N is the total shot number.
//    配列Sには，ショット位置を入れる．
//    [[0,5], [5,10]]の用になる
    std::vector<std::vector<int>> S;
    std::vector<std::vector<std::vector<int>>> save_S;
//    ショットファイルを読み込む関数
    void read_shot_file();

//    2. ショットiから等間隔でN枚のキーフレームを抽出し、ショットの内容を表現する。
//    Kの中に等間隔のキーフレームを保存する　K[K.size/2]はショットの中心になる．
//    [[0,1,2,3,4],[5,6,7,8,9]]の用になる
    void create_keyframe();
    std::vector<std::vector<std::vector<int>>> K;
    std::vector<std::vector<std::vector<std::vector<int>>>> save_K;
//    ヒストグラムを作成する
//    H
    std::vector<std::vector<std::vector<float>>> H;
    std::vector<std::vector<std::vector<std::vector<float>>>> save_H;
    void init_histogram();
    std::vector<float> create_histogram(int frame_num);
    std::vector<float> serch_histogram(int m, int i);
    
//   最短の組み合わせを求める．
    float get_histogram_distance(std::vector<std::vector<float>> hist, std::vector<std::vector<float>> hist2);
    cv::Point2i combination_min();
//    最短の二つのシーンを一つのシーンクラスタに結合する　sl = (si, sj)
    cv::Point2i marge(cv::Point2i index);
    void marge_two_scenes(int i, int j);
//    結合された二つのシーンヒストグラムを計算する
    void marge_two_hist(std::vector<double> si_hist, std::vector<double> sj_hist,  int si_scene_num, int sj_scene_num);
    
//    終わらせる計算
    void end_process(int index);
    
    float get_euclidean_distance(std::vector<float> hist, std::vector<float> hist_two);
    
    float divide = 0.0;
    float Ni = 0;
    std::vector<std::vector<std::vector<float>>> clusters;
    void update_cluster();
    std::vector<std::vector<std::vector<float>>> clusters_hist;
    void update_cluster_hist();
    
    void Jinit(int N);
    void end(cv::Point2i index);
    cv::Point2i combination_min_index(cv::Point2i index);
    
    void output(std::vector<std::vector<std::vector<float>>> cluster);
    void JbaseClustring();
    std::vector<double> Jresult;
    std::vector<std::vector<std::vector<std::vector<float>>>> save_clusters;

};

#endif
