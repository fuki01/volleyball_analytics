#include "Jclustring.hpp"


Jclustring::Jclustring(std::string video_file_path, std::string shot_path, int N){
    this->N = N;
    this->video_file_path = video_file_path;
    this->shot_path = shot_path;
}

void Jclustring::run(){
    std::chrono::system_clock::time_point  start, end;
    this->cap.open(this->video_file_path);
    this->read_shot_file();
    std::cout<< "start split_shot" << std::endl;
    this->split_shot();
    std::cout<< "end split_shot" << std::endl;
    std::cout<< "start create_key_frame_histgram" << std::endl;
    this->create_key_frame_histgram();
    std::cout<< "end create_key_frame_histgram" << std::endl;
    
    
    this->save();
    while(true){
        std::cout<< "start shot comparison" << std::endl;
        this->shot_comparison();
        std::cout<< "end shot comparison" << std::endl;
        if(this->split_shot_list.size() == 1) break;
        // textへ書き出す
        std::cout<< "start endprocess" << std::endl;
        start = std::chrono::system_clock::now(); // 計測開始時間
        this->end_process();
        end = std::chrono::system_clock::now();  // 計測終了時間
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); //処理に要した時間をミリ秒に変換
        std::cout<< "end endprocess " << elapsed << "ms" << std::endl;
        this->save_shot_list.push_back(this->split_shot_list);
    }
    this->output();
    puts("end");
}

void Jclustring::output(){
    // en_listの最小値を求める
    double min = this->en_list[0];
    for(int i = 1; i < this->en_list.size(); i++){
        if(min > this->en_list[i]) min = this->en_list[i];
    }
    // en_list minのindexを求める
    int min_index = 0;
    for(int i = 0; i < this->en_list.size(); i++){
        if(min == this->en_list[i]){
          min_index = i;
          break;
        }
    }

    // min_indexのthis->save_shot_listを求める
    std::ofstream ofs("/Users/fuki/src/j-base-clustring/output/result.txt", std::ios::app);
    for (int i=0; i<this->save_shot_list[min_index].size(); i++) {
        ofs << "=======================" << std::endl;
        for (int l=0; l<this->save_shot_list[min_index][i].size(); l++) {
            ofs << this->save_shot_list[min_index][i][l] << std::endl;
        }
    }
}

void Jclustring::end_process(){
    double dividend = 0.0;
    for(int i=0; i<this->split_shot_list.size(); i++){
        std::vector<int> tmp;
        std::vector<std::vector<int>> split_N_shot;
        for(int l=0; l<this->split_shot_list[i].size(); l++){
            tmp.push_back(this->split_shot_list[i][l]);
            if(l%this->N==this->N-1){
                split_N_shot.push_back(tmp);
                tmp = {};
            }
        }
        
        std::vector<std::vector<double>> hist_tmp;
        std::vector<std::vector<double>> hist_avg_tmp;
        double tmp_num = 0.0;
        for(int l=0; l<split_N_shot.size(); l++){
            this->cap.set(cv::CAP_PROP_POS_FRAMES, split_N_shot[l][int(this->N/2)]);
            cv::Mat frame;
            this->cap >> frame;
            std::vector<int> tmp = this->histogram_create(frame);
            std::vector<double> tmp_two;
            // 全体を１に正規化する
            // ヒストグラムの総和を求める
            double hist_sum = 0.0;
            for(int k=0; k<tmp.size(); k++) hist_sum += tmp[k];
            for(int k=0; k<tmp.size(); k++) tmp_two.push_back(tmp[k]/hist_sum);
            tmp_num += this->euclidean_distance(tmp_two, this->histogram_avarage(this->split_shot_hist_list[i]));
        }
        dividend += tmp_num;
    }
    
    
    std::cout << dividend << " / " << this->divide << std::endl;
    double jn = dividend/this->divide;
    double split_shot_list_len = this->split_shot_list.size();
    double split_shot_list_root_len = this->split_shot_list_root.size();
    double kn = split_shot_list_len/split_shot_list_root_len;
    
    std::ofstream ofs("/Users/fuki/src/j-base-clustring/output/output.txt", std::ios::app);
    ofs << kn << "," << jn << "," << kn+jn << std::endl;
    this->en_list.push_back(kn+jn);
}


double Jclustring::euclidean_distance(std::vector<double> hist, std::vector<double> hist_avg){
    double d = 0;
    for (int i=0; i<hist.size(); i++) {
        double tmp = hist[i] - hist_avg[i];
        d += tmp*tmp;
    }
    return sqrt(d);
}


std::vector<std::vector<int>> comparison;
void Jclustring::shot_comparison(){
    comparison = {};
    std::vector<std::vector<int>> tmp_one;
    foreach_comb(this->split_shot_hist_list.size(), 2, [](int *indexes) {
        comparison.push_back({indexes[0], indexes[1]});
    });
    for (int i=0; i<comparison.size(); i++) {
        int l = comparison[i][0], j =comparison[i][1];
        tmp_one.push_back({this->shot_distance(this->split_shot_hist_list[l], this->split_shot_hist_list[j]), l, j});
    }
    
    // 一番距離が小さいものを探す
    int min_index = 0;
    int min_value = tmp_one[0][0];
    for(int i=0; i<tmp_one.size(); i++){
        if(tmp_one[i][0] < min_value){
            min_index = i;
            min_value = tmp_one[i][0];
        }
    }

    std::vector<int> min_shot = tmp_one[min_index];
    std::vector<int> min_shot_key_frames_one = this->split_shot_list[min_shot[1]];
    std::vector<int> min_shot_key_frames_two = this->split_shot_list[min_shot[2]];

    min_shot_key_frames_one.insert(min_shot_key_frames_one.end(), min_shot_key_frames_two.begin(), min_shot_key_frames_two.end());
    this->split_shot_list[min_shot[1]] = min_shot_key_frames_one;
    // this->split_shot_list からindex min_shot[2] を削
    this->split_shot_list.erase(this->split_shot_list.begin() + min_shot[2]);
    // ヒストグラムを結合する
    // 最小の距離のショット1 ヒストグラム
    std::vector<std::vector<int>> min_shot_hist_one = this->split_shot_hist_list[min_shot[1]];
    std::vector<std::vector<int>> min_shot_hist_two = this->split_shot_hist_list[min_shot[2]];
    
    int scene_number_one = int(this->split_shot_list[min_shot[2]].size()/this->N);
    int scene_number_two = int(this->split_shot_list[min_shot[1]].size()/this->N);
    
    std::vector<std::vector<int>> hist_tmp;
    for(int i=0; i<min_shot_hist_one.size(); i++){
        hist_tmp.push_back(this->histogram_marge(min_shot_hist_one[i], min_shot_hist_two[i], scene_number_one, scene_number_two));
    }
    
    this->split_shot_hist_list[min_shot[1]] = hist_tmp;
    this->split_shot_hist_list.erase(this->split_shot_hist_list.begin() + min_shot[2]);
}


void Jclustring::save(){
    copy(this->split_shot_list.begin(), this->split_shot_list.end(), back_inserter(this->split_shot_list_root));
    copy(this->split_shot_hist_list.begin(), this->split_shot_hist_list.end(), back_inserter(this->split_shot_hist_list_root));
    std::vector<std::vector<double>> hist_one, hist_two;
    for(int i=0; i<this->split_shot_hist_list_root.size(); i++){
        std::vector<int> tmp = this->split_shot_hist_list_root[i][int(this->N/2)];
        std::vector<double> tmp_two;
        double hist_sum = 0.0;
        for(int k=0; k<tmp.size(); k++) hist_sum += tmp[k];
        for(int k=0; k<tmp.size(); k++){
            tmp_two.push_back(double(tmp[k])/hist_sum);
        }

        this->divide += this->euclidean_distance(tmp_two, histogram_avarage(this->split_shot_hist_list_root[i]));
    }


}
void Jclustring::read_shot_file(){
    // ファイルを読み込む
    std::ifstream ifs(this->shot_path);
    std::string line;
    std::vector<int> shot_tmp;
    while (std::getline(ifs, line)) {
        std::stringstream ss(line);
        shot_tmp.push_back(int(std::stoi(line)));
    }
    for(int i=1; i < shot_tmp.size(); i++)
        this->shot_list.push_back({shot_tmp[i-1], shot_tmp[i]});
}

void Jclustring::split_shot(){
    for(int i=0; i<this->shot_list.size(); i++){
        std::vector<int> tmp;
        tmp.push_back(this->shot_list[i][0]);
        for(int j=1; j<this->N; j++){
            tmp.push_back(this->shot_list[i][0] + int((this->shot_list[i][1] - this->shot_list[i][0])/this->N) * j);
        }
        this->split_shot_list.push_back(tmp);
    }
}

void Jclustring::create_key_frame_histgram(){
    std::cout<< "video loading..." << std::endl;
    for(int i=0; i<this->split_shot_list.size(); i++){
        std::vector<std::vector<int>> tmp;
        for(int j=0; j<this->split_shot_list[i].size(); j++){
            this->cap.set(cv::CAP_PROP_POS_FRAMES, this->split_shot_list[i][j]);
            cv::Mat frame;
            this->cap >> frame;
            tmp.push_back(this->histogram_create(frame));
        }
        this->split_shot_hist_list.push_back(tmp);
    }

}

void Jclustring::debug(){
    cv::Mat input_img;
    if(!this->cap.isOpened()){
        printf("Movie not found.");
        exit(1);
    }
    while (1) {
        cap >> input_img;
        cv::imshow("output", input_img);
        if (cv::waitKey(1) == 'q') break; //qを押すと終了
    }
}

int Jclustring::shot_distance(std::vector<std::vector<int>> shi,std::vector<std::vector<int>> shj){
    std::vector<int> distance_list;
    for(int i=0; i<shi.size(); i++){
        for(int j=0; j<shj.size(); j++){
            if(i!=j){
                distance_list.push_back(int(this->histogram_distance(shi[i], shj[j])));
            }
        }
    }
    std::sort(distance_list.begin(), distance_list.end());
    return int((int(distance_list[0]) + int(distance_list[1]))/2);
}

std::vector<int> Jclustring::histogram_marge(std::vector<int> hist, std::vector<int> hist_other, int Ni, int Nj){
    std::vector<int> result;
    for(int i=0; i<hist.size(); i++){
        result.push_back(int(((hist[i]*Ni) + (hist_other[i]*Nj))/(Ni+Nj)));
    }
    return result;
}

double Jclustring::histogram_distance(std::vector<int> shi, std::vector<int> shj){
    double sum = 0;
    
    for(int i=0; i<shi.size(); i++){
        sum += std::abs(shi[i] - shj[i]);
    }
    return sum/256;
}

std::vector<int> Jclustring::histogram_create(cv::Mat image){
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

    // S 4 bins
    cv::Mat hist_sat;
    int hist_sat_bins = 4;
    float hist_sat_range[] = { 0, 256 };
    const float* hist_sat_ranges = { hist_sat_range };
    cv::calcHist(&hsv_channels[1], 1, 0, cv::Mat(), hist_sat, 1, &hist_sat_bins, &hist_sat_ranges, true, false);
    
    // V 4 bins
    cv::Mat hist_val;
    int hist_val_bins = 4;
    float hist_val_range[] = { 0, 256 };
    const float* hist_val_ranges = { hist_val_range };
    cv::calcHist(&hsv_channels[2], 1, 0, cv::Mat(), hist_val, 1, &hist_val_bins, &hist_val_ranges, true, false);
    
    std::vector<int> tmp;
    for(int i=0; i<16; i++){
        for(int j=0; j<4; j++){
            for(int k=0; k<4; k++){
                tmp.push_back(int(hist_hue.at<float>(i) + hist_sat.at<float>(j) + hist_val.at<float>(k)));
            }
        }
    }
//    // ヒストグラムを０から１に正規化
//    for(int i=0; i<tmp.size(); i++){
//        tmp[i] = int(tmp[i]/tmp.size());
//    }
//
    
    return tmp;
}

std::vector<double> Jclustring::histogram_avarage(std::vector<std::vector<int>> hist_vec){
    std::vector<double> result(256);
    for (int i=0; i<hist_vec.size(); i++) {
        for(int j=0; j<hist_vec[i].size(); j++){
            result[j] += hist_vec[i][j];
        }
    }
    
    std::vector<double> tmp_two;
    double hist_sum = 0.0;
    for(int k=0; k<result.size(); k++) hist_sum += result[k];
    for(int k=0; k<result.size(); k++){
        tmp_two.push_back(result[k]/hist_sum);
    }
    return tmp_two;
}

void Jclustring::foreach_comb(int n, int k, std::function<void(int *)> f){
  int indexes[k];
  recursive_comb(indexes, n - 1, k, f);
}

void Jclustring::recursive_comb(int *indexes, int s, int rest, std::function<void(int *)> f){
  if (rest == 0) {
    f(indexes);
  } else {
    if (s < 0) return;
    recursive_comb(indexes, s - 1, rest, f);
    indexes[rest - 1] = s;
    recursive_comb(indexes, s - 1, rest - 1, f);
  }
}
