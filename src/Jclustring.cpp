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
    this->init_histogram();
    
    while(true) {
        this->JbaseClustring();
        int min_index = 0;
        double min_value = 1000;
        for (int i=0; i<this->Jresult.size(); i++) {
            if(min_value > this->Jresult[i]){
                min_value = this->Jresult[i];
                min_index = i;
            }
        }
        this->Jresult.clear();
        std::cout << min_value << ", " << min_index << std::endl;
        std::vector<std::vector<std::vector<float>>> cluster = this->save_clusters[min_index];
        this->output(cluster);
//        最適なクラスタ位置へS,H,Kを戻す．
//        メモリ解放
        S.clear();
//        メモリ確保
        for (int i=0; i<save_S[min_index].size(); i++) {
            std::vector<int> tmp;
            for (int j=0; j<save_S[min_index][i].size(); j++) tmp.push_back('null');
            S.push_back(tmp);
        }
//        コピー
        copy(save_S[min_index].begin(), save_S[min_index].end(), S.begin());
//        メモリ解放
        H.clear();
//        メモリ確保
        for (int i=0; i<save_H[min_index].size(); i++) {
            std::vector<std::vector<float>> tmp_two;
            for (int j=0; j<save_H[min_index][i].size(); j++) {
                std::vector<float> tmp_one;
                for (int h=0; h<save_H[min_index][i][j].size(); h++) tmp_one.push_back('null');
                tmp_two.push_back(tmp_one);
            }
            H.push_back(tmp_two);
        }
//        コピー
        copy(save_H[min_index].begin(), save_H[min_index].end(), H.begin());

//        vecメモリの解放
        K.clear();
//        コピーするメモリ確保
        for (int i=0; i<save_K[min_index].size(); i++) {
            std::vector<std::vector<int>> tmp_two;
            for (int j=0; j<save_K[min_index][i].size(); j++) {
                std::vector<int> tmp_one;
                for (int l=0; l<save_K[min_index][j].size(); l++) {
                    tmp_one.push_back('null');
                }
                tmp_two.push_back(tmp_one);
            }
            K.push_back(tmp_two);
        }
//        コピー
        copy(save_K[min_index].begin(), save_K[min_index].end(), K.begin());
//        デバッグ用
        std::cout << H.size() << " " << S.size() << " " << K.size() << std::endl;
        this->clusters.clear();
        this->save_clusters.clear();
    }
}

void Jclustring::JbaseClustring(){
    cv::Point2i index = this->combination_min();
    this->Jinit(K.size());
    this->Ni = this->K.size();
    int i = 0;
    while (true) {
        i++;
        this->end(index);
        index = this->marge(index);
        this->update_cluster();
//        std::cout << this->S.size() << std::endl;
        if (this->S.size() == 1) break;
        if (this->S.size() == 50) break;
        std::cout << this->S.size() << std::endl;
        index = this->combination_min_index(index);
        this->save_clusters.push_back(this->clusters);
        this->save_S.push_back(this->S);
        this->save_H.push_back(this->H);
        this->save_K.push_back(this->K);
    }
}

void Jclustring::output(std::vector<std::vector<std::vector<float>>> cluster){
    for (int i=0; i<cluster.size(); i++) {
        std::cout << "[" ;
        for (int j=0; j<cluster[i].size(); j++){
            if (j == cluster.size()) {
                std::cout << cluster[i][j][0];
            }else{
                std::cout << cluster[i][j][0] << ",";
            }
        }
        std::cout << "],";
    }
}

void Jclustring::end(cv::Point2i index){
    double divided  = 0.0;
    int index_one = index.x;
    int index_two = index.y;

    std::vector<std::vector<std::vector<int>>> tmp;
    tmp.push_back(K[index_one]);
    tmp.push_back(K[index_two]);
    for (int c=0; c<tmp.size(); c++) {
        double sum = 0.0;
        for (int i=0; i<tmp[c].size(); i++) {
            std::vector<float> center_hist = this->create_histogram(tmp[c][i][3]);
            std::vector<float> avg_hist(256);
            for (int l=0; l<5; l++) {
                std::vector<float> hist = this->create_histogram(tmp[c][i][l]);
                for (int h=0; h<256; h++) avg_hist[h] += hist[h];
            }
            for (int h=0; h<256; h++) avg_hist[h] /= 5;
            sum += this->get_euclidean_distance(center_hist, avg_hist);
        }
        divided += sum;
    }

    float kn = this->S.size() / this->Ni;
//    std::cout << divided/this->divide << " " << kn << " " << divided/this->divide+kn  << std::endl;
    this->Jresult.push_back(divided/this->divide+kn);
}

void Jclustring::Jinit(int N){
    this->divide = 0;
//    Nはショット数
    for (int i=0; i<N; i++) {
//        ショットの中心ヒストグラム
        std::vector<float> center_hist = H[i][3];
//        ショットの平均ヒストグラム
        std::vector<float> avg_hist(256);
        for (int h=0; h<H[i].size(); h++) for (int j=0; j<256; j++) avg_hist[j] += H[i][h][j];
        for (int j=0; j<256; j++) avg_hist[j] /= H[i].size();
//        距離を求める
        this->divide += this->get_euclidean_distance(center_hist, avg_hist);
    }
}

void Jclustring::end_process(int index){
    double divided = 0;
//    for (int c=0; c<K[index].size(); c++) {
//        std::vector<float> center_hist = this->create_histogram(K[index][c][3]);
////        ヒストグラム平均
//        std::vector<float> avg_hist(256);
//        for (int i=0; i<K[index][c].size(); i++) {
//            std::vector<float> hist = this->create_histogram(K[index][c][i]);
//            for (int l=0; l<256; l++) avg_hist[l] = avg_hist[l] + hist[l];
//        }
//        for (int i=0; i<256; i++) avg_hist[i] = avg_hist[i] / K[index][c].size();
//        divided += this->get_euclidean_distance(center_hist, avg_hist);
//    }
    for (int c=0; c<S.size(); c++) {
        double sum = 0.0;
        for (int i=0; i<this->K[c].size(); i++) {
            std::vector<float> center_hist = this->create_histogram(K[c][i][3]);
            std::vector<float> avg_hist(256);
            for (int h=0; h<K[c][i].size(); h++) {
                std::vector<float> hist = this->create_histogram(K[c][i][h]);
                for (int l=0; l<256; l++) avg_hist[l] = avg_hist[l] + hist[l];
            }
            for (int h=0; h<256; h++) avg_hist[h] = avg_hist[h] / this->N;
            sum += this->get_euclidean_distance(center_hist, avg_hist);
        }
        divided += sum;
    }


//
//    float kn = this->S.size() / this->Ni;
//    std::cout << divided/this->divide << " " << kn << " " << (divided/this->divide)+kn  << std::endl;
    
//    多分これが正しい
//    for (int c=0; c<K.size(); c++) {
//        double tmp = 0.0;
//        for (int i=0; i<K[c].size(); i++) {
//            int center_frame_num = K[c][i][3];
//            std::vector<float> center_hist = this->create_histogram(center_frame_num);
//            std::vector<float> avg_hist(256);
//            for (int j=0; j<H[c].size(); j++) for (int h=0; h<256; h++) avg_hist[h] += H[c][j][h];
//            for (int h=0; h<256; h++) avg_hist[h] = avg_hist[h] / this->N;
//            tmp += this->get_euclidean_distance(center_hist, avg_hist);
//        }
//        divided += tmp;
//    }
    float kn = this->S.size() / this->Ni;
    std::cout << divided/this->divide << " " << kn << " " << divided/this->divide+kn  << std::endl;
    this->Jresult.push_back(((divided/this->divide)-1)+kn);
}

void Jclustring::update_cluster(){
    //    クラスター数を計算する　クラスタのみ抽出
    this->clusters = {};
    for (int c=0; c<K.size(); c++) {
            std::vector<std::vector<float>> tmp_two;
            for (int i=0; i<K[c].size(); i++) {
                std::vector<float> tmp;
                for (int j=0; j<K[c][i].size(); j++) {
                    tmp.push_back(K[c][i][j]);
                }
                tmp_two.push_back(tmp);
            }
            this->clusters.push_back(tmp_two);
    }
}

float Jclustring::get_euclidean_distance(std::vector<float> hist, std::vector<float> hist_two){
    float d = 0.0;
    for(int i = 0; i < hist.size(); i++) d += pow(hist[i] - hist_two[i], 2.0);
    return sqrt(d);
}

cv::Point2i Jclustring::marge(cv::Point2i index){
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
    if (index_one > index_two) {
        index.x = index.x - 1;
        return index;
    }
    return index;
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
    for (int i=1; i<tmp.size(); i++) this->S.push_back({tmp[i-1]+1, tmp[i]-1});
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
            if (i!=j and H[i].size()==1 and H[j].size()==1) {
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


cv::Point2i Jclustring::combination_min_index(cv::Point2i index){
    float min = 1000000;
    cv::Point2d result = {0.0, 0.0};

    for (int i=0; i<this->H.size(); i++) {
        if(index.x != i){
            std::vector<std::vector<float>> hist_one = this->H[i];
            std::vector<std::vector<float>> hist_two = this->H[index.x];
            float dst = this->get_histogram_distance(hist_one, hist_two);
            if (min > dst) {
                min = dst;
                result.x = index.x;
                result.y = i;
            }
        }
    }
    return result;
}
