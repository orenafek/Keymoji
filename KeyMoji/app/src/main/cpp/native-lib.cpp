#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "openface/LandmarkDetector/include/LandmarkCoreIncludes.h"
#include "opencv2/ml/ml.hpp"


#include "openface/FaceAnalyser/include/FaceAnalyser.h"
#include <fstream>
#include <android/log.h>

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

    for (auto i : regs) {
        __android_log_print(ANDROID_LOG_INFO, "Daniel regs", "regs: %lf, ", i);
    }
    for (auto i : classes) {
        __android_log_print(ANDROID_LOG_INFO, "Daniel regs", "classes: %lf, ", i);
    }
//    __android_log_print(ANDROID_LOG_INFO,"Daniel regs","%s",regs_string.c_str());
//    __android_log_print(ANDROID_LOG_INFO,"Daniel classes","%s",classes_string.c_str());
    int prediction = tree(regs, classes);

    return prediction;
}

bool isDefualt(vector<double> v_r, vector<double> v_c) {

    for (auto i : v_r) {
        if (i != 0)
            return false;
    }
    for (auto i : v_c) {
        if (i != 0)
            return false;
    }
    __android_log_print(ANDROID_LOG_INFO, "Defualt", "didnt recognize a face");

    return true;
}

int tree(vector<double> v_r, vector<double> v_c) {
    if (isDefualt(v_r, v_c)) return 2;
    if (v_r[8] <= 1.11750006676) {
        if (v_r[2] <= 1.25100004673) {
            if (v_r[3] <= 0.5) {
                if (v_r[7] <= 0.115950003266) {
                    if (v_r[4] <= 0.252750009298) {
                        if (v_r[2] <= 0.5) {
                            if (v_r[9] <= 0.0553999990225) {
                                if (v_r[5] <= 0.141100004315) {
                                    return 1;
                                } else { // if(v_r[5] > 0.141100004315)
                                    if (v_r[9] <= 0.5) {
                                        return 4;
                                    } else { // if(v_r[9] > 0.5)
                                        return 1;
                                    }
                                }
                            } else { // if(v_r[9] > 0.0553999990225)
                                return 3;
                            }
                        } else { // if(v_r[2] > 0.5)
                            return 3;
                        }
                    } else { // if(v_r[4] > 0.252750009298)
                        if (v_r[2] <= 1.02939999104) {
                            return 1;
                        } else { // if(v_r[2] > 1.02939999104)
                            return 3;
                        }
                    }
                } else { // if(v_r[7] > 0.115950003266)
                    return 3;
                }
            } else { // if(v_r[3] > 0.5)
                if (v_c[9] <= 0.0370949991047) {
                    if (v_c[9] <= 0.5) {
                        if (v_c[5] <= 0.191500008106) {
                            if (v_c[5] <= 0.0455449968576) {
                                if (v_c[7] <= 0.5) {
                                    if (v_c[5] <= 0.5) {
                                        if (v_c[16] <= 0.5) {
                                            if (v_c[2] <= 0.996100008488) {
                                                if (v_c[5] <= 0.0146749997512) {
                                                    return 4;
                                                } else { // if(v_c[5] > 0.0146749997512)
                                                    if (v_c[5] <= 0.0304049998522) {
                                                        return 3;
                                                    } else { // if(v_c[5] > 0.0304049998522)
                                                        return 4;
                                                    }
                                                }
                                            } else { // if(v_c[2] > 0.996100008488)
                                                if (v_c[2] <= 1.14199995995) {
                                                    return 3;
                                                } else { // if(v_c[2] > 1.14199995995)
                                                    return 4;
                                                }
                                            }
                                        } else { // if(v_c[16] > 0.5)
                                            if (v_c[2] <= 0.411549985409) {
                                                return 3;
                                            } else { // if(v_c[2] > 0.411549985409)
                                                return 3;
                                            }
                                        }
                                    } else { // if(v_c[5] > 0.5)
                                        return 2;
                                    }
                                } else { // if(v_c[7] > 0.5)
                                    return 3;
                                }
                            } else { // if(v_c[5] > 0.0455449968576)
                                return 3;
                            }
                        } else { // if(v_c[5] > 0.191500008106)
                            if (v_c[8] <= 0.00827000010759) {
                                return 4;
                            } else { // if(v_c[8] > 0.00827000010759)
                                if (v_c[8] <= 0.0933299958706) {
                                    return 3;
                                } else { // if(v_c[8] > 0.0933299958706)
                                    return 4;
                                }
                            }
                        }
                    } else { // if(v_c[9] > 0.5)
                        return 1;
                    }
                } else { // if(v_c[9] > 0.0370949991047)
                    if (v_c[4] <= 0.243950009346) {
                        if (v_c[9] <= 0.559800028801) {
                            if (v_c[5] <= 0.5) {
                                return 3;
                            } else { // if(v_c[5] > 0.5)
                                return 1;
                            }
                        } else { // if(v_c[9] > 0.559800028801)
                            if (v_c[16] <= 0.5) {
                                return 4;
                            } else { // if(v_c[16] > 0.5)
                                if (v_c[8] <= 0.127100005746) {
                                    return 1;
                                } else { // if(v_c[8] > 0.127100005746)
                                    return 3;
                                }
                            }
                        }
                    } else { // if(v_c[4] > 0.243950009346)
                        if (v_c[2] <= 0.130400002003) {
                            return 1;
                        } else { // if(v_c[2] > 0.130400002003)
                            return 4;
                        }
                    }
                }
            }
        } else { // if(v_r[2] > 1.25100004673)
            if (v_c[4] <= 0.5) {
                if (v_c[9] <= 0.5) {
                    if (v_c[5] <= 0.0465499982238) {
                        return 3;
                    } else { // if(v_c[5] > 0.0465499982238)
                        if (v_c[9] <= 0.494100004435) {
                            if (v_c[5] <= 2.35300016403) {
                                if (v_c[2] <= 2.29550004005) {
                                    if (v_c[5] <= 0.5) {
                                        return 3;
                                    } else { // if(v_c[5] > 0.5)
                                        if (v_c[2] <= 1.96650004387) {
                                            return 1;
                                        } else { // if(v_c[2] > 1.96650004387)
                                            if (v_c[13] <= 0.5) {
                                                return 1;
                                            } else { // if(v_c[13] > 0.5)
                                                return 3;
                                            }
                                        }
                                    }
                                } else { // if(v_c[2] > 2.29550004005)
                                    return 3;
                                }
                            } else { // if(v_c[5] > 2.35300016403)
                                return 1;
                            }
                        } else { // if(v_c[9] > 0.494100004435)
                            return 1;
                        }
                    }
                } else { // if(v_c[9] > 0.5)
                    return 1;
                }
            } else { // if(v_c[4] > 0.5)
                return 1;
            }
        }
    } else { // if(v_r[8] > 1.11750006676)
        if (v_c[7] <= 2.91000008583) {
            return 2;
        } else { // if(v_c[7] > 2.91000008583)
            return 1;
        }
    }
    return -1;
}


extern "C" {
JNIEXPORT jint

JNICALL
Java_il_ac_technion_gip_keymoji_KeyMojiIME_getEmotion(JNIEnv *env, jobject instance,
                                                      jlong matAddrGray) {
    return getAUs(matAddrGray);

}

}

extern "C" {
JNIEXPORT jint
JNICALL
Java_il_ac_technion_gip_keymoji_KeyMojiIME_A(JNIEnv *env, jobject instance, jlong add) {

    return getAUs(add);

}
}

extern "C" {
JNIEXPORT jint
JNICALL
Java_il_ac_technion_gip_keymoji_MainActivity_getEmotion(JNIEnv *env, jobject instance,
                                                        jlong matAddrGray) {

    return getAUs(matAddrGray);

}
}