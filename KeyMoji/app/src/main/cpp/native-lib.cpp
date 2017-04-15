#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "openface/LandmarkDetector/include/LandmarkCoreIncludes.h"
#include "opencv2/ml/ml.hpp"


#include "openface/FaceAnalyser/include/FaceAnalyser.h"
#include <fstream>

#define  LOG_TAG    "ndk-tag"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
using namespace std;
using namespace cv;
using namespace cv::ml;
#define PACKAGE il.ac.technion.gip.keymoji
#define PATH "data/data/il.ac.technion.gip.keymoji"

string concat(const string &s1, const string &s2) {
    stringstream ss;
    ss << s1 << s2;
    return ss.str();
}

string fullPath(const string &relativePath) {
    return concat(PATH, concat("/", relativePath));
}

int tree(vector<double> v_r, vector<double> v_c);

vector<double> getSeconds(vector<std::pair<string, vector<double>>> v) {
    vector<double> result;
    for (std::pair<string, vector<double>> p : v) {
        assert(p.second.size() == 1);
        result.push_back(p.second[0]);
    }
    return result;
};

string getStrings(vector<std::pair<string, vector<double>>> v) {
    string s = "";
    for (std::pair<string, vector<double>> p : v) {
        assert(p.second.size() == 1);
        s += p.first + ", ";
    }
    return s.substr(0, s.size() - 1);
};

int getAUs(jlong matAddrGray) {
    Mat &captured_image = *(Mat *) matAddrGray;
    string main_clnf_general = "data/data/il.ac.technion.gip.keymoji/model/main_clnf_general.txt";
    string face_detector_location = concat(PATH, "/classifiers/haarcascade_frontalface_alt.xml");


    string au_loc = "data/data/il.ac.technion.gip.keymoji/AU_predictors/AU_all_best.txt";
    string tri_loc = "data/data/il.ac.technion.gip.keymoji/model/tris_68_full.txt";


    auto v = vector<cv::Vec3d>();
    FaceAnalysis::FaceAnalyser face_analyser(v, 0.7, 112, 112, au_loc, tri_loc);
    Mat grayscale_image = captured_image.clone();
    LandmarkDetector::FaceModelParameters det_parameters;
    det_parameters.init();
    det_parameters.model_location = main_clnf_general;
    det_parameters.face_detector_location = face_detector_location;

    LandmarkDetector::CLNF face_model;
    face_model.inits(det_parameters);
    face_model.face_detector_HAAR = CascadeClassifier(face_detector_location);
    bool loadedSuccess = face_model.face_detector_HAAR.load(face_detector_location);

    LandmarkDetector::DetectLandmarksInImage(grayscale_image, face_model, det_parameters);
    int time_stamp = 4;
    face_analyser.AddNextFrame(captured_image, face_model, time_stamp, false,
                               true);// last parameter is quiet mode inverted !det_parameters.quiet_mode

    bool dynamic = true;
    vector<double> certainties;
    vector<bool> successes;
    vector<double> timestamps;
    vector<std::pair<std::string, vector<double>>> predictions_reg;
    vector<std::pair<std::string, vector<double>>> predictions_class;
    face_analyser.ExtractAllPredictionsOfflineReg(predictions_reg, certainties, successes,
                                                  timestamps, dynamic);
    face_analyser.ExtractAllPredictionsOfflineClass(predictions_class, certainties, successes,
                                                    timestamps, dynamic);

    string regs_string = getStrings(predictions_reg);
    string classes_string = getStrings(predictions_class);

    vector<double> regs = getSeconds(predictions_reg);
    vector<double> classes = getSeconds(predictions_class);

    int prediction = tree(regs, classes);

    return prediction;
}


int tree(vector<double> v_r, vector<double> v_c) {
    if (v_r[8] <= 1.0120500326156616) {
        if (v_r[7] <= 1.2769999504089355) {
            if (v_r[2] <= 1.8274999856948853) {
                if (v_r[4] <= 0.6072999835014343) {
                    if (v_r[9] <= 0.023579999804496765) {
                        if (v_r[4] <= 0.4477499723434448) {
                            if (v_r[9] <= 0.002859999891370535) {
                                if (v_r[16] <= 0.5) {
                                    if (v_r[5] <= 0.5) {
                                        if (v_r[5] <= 0.22220000624656677) {
                                            return 0;
                                        } else { // if(v_r[5] > 0.22220000624656677)
                                            return 5;
                                        }
                                    } else { // if(v_r[5] > 0.5)
                                        return 3;
                                    }
                                } else { // if(v_r[16] > 0.5)
                                    if (v_r[2] <= 1.142699956893921) {
                                        return 6;
                                    } else { // if(v_r[2] > 1.142699956893921)
                                        return 5;
                                    }
                                }
                            } else { // if(v_r[9] > 0.002859999891370535)
                                return 2;
                            }
                        } else { // if(v_r[4] > 0.4477499723434448)
                            return 2;
                        }
                    } else { // if(v_r[9] > 0.023579999804496765)
                        if (v_c[4] <= 0.5) {
                            if (v_c[2] <= 0.690750002861023) {
                                if (v_c[9] <= 0.3237999975681305) {
                                    if (v_c[9] <= 0.2062000036239624) {
                                        return 5;
                                    } else { // if(v_c[9] > 0.2062000036239624)
                                        return 2;
                                    }
                                } else { // if(v_c[9] > 0.3237999975681305)
                                    return 5;
                                }
                            } else { // if(v_c[2] > 0.690750002861023)
                                if (v_c[2] <= 0.9946500062942505) {
                                    return 6;
                                } else { // if(v_c[2] > 0.9946500062942505)
                                    return 5;
                                }
                            }
                        } else { // if(v_c[4] > 0.5)
                            return 0;
                        }
                    }
                } else { // if(v_r[4] > 0.6072999835014343)
                    if (v_c[9] <= 0.8761000037193298) {
                        return 6;
                    } else { // if(v_c[9] > 0.8761000037193298)
                        return 0;
                    }
                }
            } else { // if(v_r[2] > 1.8274999856948853)
                return 0;
            }
        } else { // if(v_r[7] > 1.2769999504089355)
            return 1;
        }
    } else { // if(v_r[8] > 1.0120500326156616)
        if (v_c[2] <= 0.31985002756118774) {
            return 3;
        } else { // if(v_c[2] > 0.31985002756118774)
            if (v_c[5] <= 3.242499828338623) {
                return 4;
            } else { // if(v_c[5] > 3.242499828338623)
                return 3;
            }
        }
    }
    return -1;
}

extern "C" {
JNIEXPORT jint JNICALL
Java_il_ac_technion_gip_keymoji_MainActivity_getEmotion(JNIEnv *env, jobject instance,
                                                        jlong matAddrGray) {

    return getAUs(matAddrGray);

}
}