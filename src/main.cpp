#include <iostream>
#include <opencv2/opencv.hpp>
#include "Jclustring.hpp"
using namespace std;

//const std::string movie_path = "/Users/fuki/Desktop/volleballAnalysis/jbase_clustring/video.mp4";
//const std::string shot_path = "/Users/fuki/Desktop/volleballAnalysis/jbase_clustring/Cython/input/vide_shot_file.csv";


//const std::string movie_path = "/Users/fuki/Documents/実験動画/womens\ u20\ volleyball\ world\ chanps\ 2021\ video\ yolo/ARG\ vs\ THA\ -\ Full\ Match\ Round\ 2\ Womens\ U20\ Volleyball\ World\ Champs\ 2021.mp4";
//const std::string shot_path = "/Users/fuki/Desktop/volleballAnalysis/shot_detection/result/ARG\ vs\ THA\ -\ Full\ Match\ Round\ 2\ Womens\ U20\ Volleyball\ World\ Champs\ 2021.mp4cut_frame.csv";


const std::string movie_path = "/Users/fuki/Desktop/video_oneset.mp4";
const std::string shot_path = "/Users/fuki/Desktop/video_oneset.csv";


int main(int argc, const char * argv[]) {
    Jclustring jclustring(movie_path, shot_path, 11);
    jclustring.run();
}
