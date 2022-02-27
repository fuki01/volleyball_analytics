#include "Jclustring.hpp"

Jclustring::Jclustring(std::string video_file_path, std::string shot_path, int N){
    this->N = N;
    this->shot_path = shot_path;
    this->video_file_path = video_file_path;
}

//全部を実行させるやつ．
void Jclustring::run(){
    bool t = this->set_video();
    if(!t) return 0;
    this->read_shot_file();
    this->create_keyframe();
    std::cout << "start init_histogram" << std::endl;
    this->init_histogram();
    std::cout << "end init_histogram" << std::endl;
    std::cout << "start combination_min" << std::endl;
    cv::Point2i index = this->combination_min();
    std::cout << "end combination_min" << std::endl;
    
    int N = this->K.size();
    this->Ni = this->K.size();
    for (int i=0; i<N; i++) {
        std::vector<float> center_hist = this->create_histogram(K[i][0][3]);
        std::vector<float> avg_hist(256);
        for (int l=0; l<5; l++) {
            std::vector<float> hist = this->create_histogram(K[i][0][l]);
            for (int j=0; j<256; j++) avg_hist[j] += hist[j];
        }
        for (int j=0; j<256; j++) avg_hist[j] = avg_hist[j] / this->N;
        this->divide += this->get_euclidean_distance(avg_hist, center_hist);
    }
    this->marge(index);
    this->update_cluster();
    int i = 0;
    while (true) {
        i++;
        cv::Point2i index = this->combination_min();
        this->marge(index);
        this->update_cluster();
        this->end_process(index.x);
//        std::cout << i << std::÷endl;
//        if (i == 17) break;
        if (this->S.size() == 1) break;

    }
    
    for (int i=0; i<this->clusters.size(); i++) {
        for (int j=0; j<clusters[i].size(); j++) std::cout << clusters[i][j][0] << "〜" << clusters[i][j][4] << " | ";
        std::cout << std::endl;
    }
}

void Jclustring::end_process(int index){
    float divided = 0;
    for (int c=0; c<this->clusters.size(); c++) {
        float sum = 0.;
        for (int i=0; i<clusters[c].size(); i++) {
            std::vector<float> center_hist = create_histogram(clusters[c][i][3]);
            std::vector<float> avg_hist(256);
            
            for (int l=0; l<5; l++) {
                std::vector<float> hist = this->create_histogram(clusters[c][i][l]);
                for (int j=0; j<256; j++) avg_hist[j] += hist[j];
            }
            for (int j=0; j<256; j++) avg_hist[j] = avg_hist[j] / this->N;
            sum += this->get_euclidean_distance(avg_hist, center_hist);
        }
        divided += sum;
    }
    
    float kn = this->clusters.size() / this->Ni;
    std::cout << divided/this->divide << " " << kn << std::endl;
}

void Jclustring::update_cluster(){
    //    クラスター数を計算する　クラスタのみ抽出
    this->clusters = {};
    for (int c=0; c<K.size(); c++) {
//        if (K[c].size() != 1){
            std::vector<std::vector<float>> tmp_two;
            for (int i=0; i<K[c].size(); i++) {
                std::vector<float> tmp;
                for (int j=0; j<K[c][i].size(); j++) {
                    tmp.push_back(K[c][i][j]);
                }
                tmp_two.push_back(tmp);
            }
            this->clusters.push_back(tmp_two);
//        }
    }
    
    //    for (int i=0; i<this->clusters.size(); i++) {
    //        std::cout << "clusters[i] " << clusters[i].size() << std::endl;
    //        for (int j=0; j<clusters[i].size(); j++) {
    //            std::cout << "clusters[i][j] " << clusters[i][j].size() << std::endl;
    //        }
    //    }
}

float Jclustring::get_euclidean_distance(std::vector<float> hist, std::vector<float> hist_two){
    float d = 0.0;
    for(int i = 0; i < hist.size(); i++) d += pow(hist[i] - hist_two[i], 2.0);
    return sqrt(d);
}

void Jclustring::marge(cv::Point2i index){
    int index_one = index.x;
    int index_two = index.y;
    int N_i = this->S[index_one].size()/2; //iクラスタのショット数
    int N_j = this->S[index_two].size()/2; //jクラスタのショットの数
    //    ヒストグラムの更新
    std::vector<std::vector<float>> new_hist;
    
    for (int i=0; i<this->N; i++) {
        std::vector<float> tmp;
        for (int l=0; l<256; l++)
            tmp.push_back(((this->H[index_one][i][l] * float(N_i)) + (this->H[index_two][i][l] * float(N_j))) / float(N_i + N_j));
        new_hist.push_back(tmp);
    }
    this->H[index_one] = new_hist;
    H.erase(H.begin() + index_two);
    
    //  ショットの更新
    this->S[index_one].insert(this->S[index_one].end(), this->S[index_two].begin(), this->S[index_two].end());
    S.erase(S.begin() + index_two);
    
    //  キーフレームの更新
    this->K[index_one].insert(this->K[index_one].end(), this->K[index_two].begin(), this->K[index_two].end());
    K.erase(K.begin() + index_two);
}


void Jclustring::init_histogram(){
    for (int i=0; i<this->K.size(); i++) {
        for (int j=0; j<this->K[i].size(); j++) {
            std::vector<std::vector<float>> tmp;
            for (int l=0; l<this->N; l++) {
                tmp.push_back(this->create_histogram(this->K[i][j][l]));
            }
            this->H.push_back(tmp);
        }
    }
}

bool Jclustring::set_video(){
    cv::VideoCapture cap(this->video_file_path);
    this->cap = cap;
    if(!cap.isOpened()){ //エラー処理
        std::cout << "video.error" << std::endl;
        return false;
    }
    return true;
}

//    ショットファイルを読み込む関数
void Jclustring::read_shot_file(){
    std::string filename(this->shot_path);
    std::ifstream input_file(filename);
    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - '"
        << filename << "'" << std::endl;
        return EXIT_FAILURE;
    }
    int number = 0;
    std::vector<int> tmp;
    while (input_file >> number) tmp.push_back(int(number));
    for (int i=1; i<tmp.size(); i++) this->S.push_back({tmp[i-1], tmp[i]-1});
    input_file.close();
}


//    [[0,1,2,3,4],[5,6,7,8,9]]の用になる
void Jclustring::create_keyframe(){
    std::vector<std::vector<int>> shot = this->S;
    for (int i=0; i<shot.size(); i++) {
        int total_frame_num = shot[i][1] - shot[i][0];
        int split_frame_num = total_frame_num / this->N;
        std::vector<int> tmp;
        for (int j=0; j<this->N; j++) {
            tmp.push_back(shot[i][0]+split_frame_num*j);
        }
        this->K.push_back({tmp});
        
    }
}

//    ヒストグラムを作成する
std::vector<float> Jclustring::create_histogram(int frame_num){
    cv::Mat image;
    this->cap.set(cv::CAP_PROP_POS_FRAMES, frame_num);
    this->cap >> image;
    cv::Mat hsv_image;
    cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV_FULL);
    // HSVを分離
    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv_image, hsv_channels);
    // ヒストグラムを作成する　H 16 bins
    cv::Mat hist_hue;
    int hist_hue_bins = 16;
    float hist_hue_range[] = { 0, 256 };
    const float* hist_hue_ranges = { hist_hue_range };
    cv::calcHist(&hsv_channels[0], 1, 0, cv::Mat(), hist_hue, 1, &hist_hue_bins, &hist_hue_ranges, true, false);
    // 正規化する
    cv::normalize(hist_hue, hist_hue, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    
    // S 4 bins
    cv::Mat hist_sat;
    int hist_sat_bins = 4;
    float hist_sat_range[] = { 0, 256 };
    const float* hist_sat_ranges = { hist_sat_range };
    cv::calcHist(&hsv_channels[1], 1, 0, cv::Mat(), hist_sat, 1, &hist_sat_bins, &hist_sat_ranges, true, false);
    // 正規化する
    cv::normalize(hist_sat, hist_sat, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    
    // V 4 bins
    cv::Mat hist_val;
    int hist_val_bins = 4;
    float hist_val_range[] = { 0, 256 };
    const float* hist_val_ranges = { hist_val_range };
    cv::calcHist(&hsv_channels[2], 1, 0, cv::Mat(), hist_val, 1, &hist_val_bins, &hist_val_ranges, true, false);
    // 正規化する
    cv::normalize(hist_val, hist_val, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    
    std::vector<float> tmp;
    for(int i=0; i<16; i++){
        for(int j=0; j<4; j++){
            for(int k=0; k<4; k++){
                tmp.push_back(hist_hue.at<float>(i) + hist_sat.at<float>(j) + hist_val.at<float>(k));
            }
        }
    }
    return tmp;
}

float Jclustring::get_histogram_distance(std::vector<std::vector<float>> hist, std::vector<std::vector<float>> hist2){
    std::vector<float> distance_list;
    for (int i=0; i<hist.size(); i++) {
        for (int j=i; j<hist.size(); j++) {
            float sum = 0;
            for (int l=0; l<hist[j].size(); l++) sum += abs(hist[j][l]-hist2[i][l]);
            distance_list.push_back(sum/256);
        }
    }
    // 一番小さい値を返す
    float min = distance_list[0];
    for (int i=0; i<distance_list.size(); i++) {
        if (min > distance_list[i]) min = distance_list[i];
    }
    // 二番目に小さい値を返す
    float second_min = distance_list[0];
    for (int i=0; i<distance_list.size(); i++) {
        if (second_min > distance_list[i] && second_min != min) second_min = distance_list[i];
    }
    return (min+second_min)/2;
}

//  ヒストグラムを検索する
std::vector<float> Jclustring::serch_histogram(int m, int i){
}
//   最短の組み合わせを求める．
cv::Point2i Jclustring::combination_min(){
    float min = 1000000;
    cv::Point2d result = {0.0, 0.0};
    for (int i=0; i<this->H.size(); i++) {
        for (int j=i; j<this->H.size(); j++) {
            if (i!=j) {
                std::vector<std::vector<float>> hist_one = this->H[i];
                std::vector<std::vector<float>> hist_two = this->H[j];
                float dst = this->get_histogram_distance(hist_one, hist_two);
                if (min > dst) {
                    min = dst;
                    result.x = i;
                    result.y = j;
                }
            }
        }
    }
    return result;
}

//    最短の二つのシーンを一つのシーンクラスタに結合する　sl = (si, sj)
void Jclustring::marge_two_scenes(int i, int j){
    
}

//    結合された二つのシーンヒストグラムを計算する
void Jclustring::marge_two_hist(std::vector<double> si_hist, std::vector<double> sj_hist,  int si_scene_num, int sj_scene_num){
    
}

