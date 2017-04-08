#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "openface/LandmarkDetector/include/LandmarkCoreIncludes.h"
#include "opencv2/ml/ml.hpp"
#include <android/log.h>


#include "openface/FaceAnalyser/include/FaceAnalyser.h"
#include <fstream>

#define  LOG_TAG    "ndk-tag"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
using namespace std;
using namespace cv;
using namespace cv::ml;
#define PACKAGE ch.hepia.iti.opencvnativeandroidstudio
#define PATH "data/data/ch.hepia.iti.opencvnativeandroidstudio"

string concat(const string &s1, const string &s2) {
    stringstream ss;
    ss << s1 << s2;
    return ss.str();
}

string fullPath(const string &relativePath) {
    return concat(PATH, concat("/", relativePath));
}

void initAllSamples();

void initTests();

int tree(vector<double> v_r, vector<double> v_c);

void initAllData() {
    initAllSamples();
    initTests();
}

vector<vector<double>> allSamples;
vector<double> testFromTrainingData;
vector<double> testHappy;
vector<double> testHappy2;

float predict(Ptr<DTrees> &classifier, vector<double> &samples) {

    Mat mat = Mat(1, samples.size(), CV_32FC1);
    for (int i = 0; i < samples.size(); ++i) {
        mat.at<double>(0, i) = samples[i];
    }
    InputArray inputArray = _InputArray(mat);
    return classifier->predict(inputArray);
}

Ptr<DTrees> &train(Ptr<DTrees> &classifier, vector<vector<double>> &samples) {

    //classifier->setMaxDepth(10);
    classifier->setMinSampleCount(2);
    //classifier->setRegressionAccuracy(0);
    classifier->setUseSurrogates(false);
    classifier->setMaxCategories(6);
    /*classifier->setPriors(Mat());
    classifier->setCalculateVarImportance(true);
    classifier->setActiveVarCount(0);
    classifier->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 0));*/

    Mat m_samples = Mat(samples.size(), samples[0].size(), CV_32FC1);

    for (int i = 0; i < samples.size(); ++i) {
        for (int j = 0; j < samples[i].size(); ++j) {
            m_samples.at<double>(i, j) = samples[i][j];
        }
    }

    vector<int> all_responses = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6};
    Mat m_responses = Mat(samples.size(), 1, CV_32S);

    for (int i = 0; i < samples.size(); ++i) {
        m_responses.at<int>(i) = all_responses[i];
    }

    InputArray samplesInputArray = _InputArray(m_samples);
    InputArray labelsInputArray = _InputArray(m_responses);
    try {
        classifier->train(samplesInputArray, 0, labelsInputArray);
    } catch (Exception &ex) {
        cout << "";
    }

    catch (...) {
        cout << "";
    }
    string modelFileName = "data/data/ch.hepia.iti.opencvnativeandroidstudio/classifiers/ourModel.xml";

    //classifier->save(modelFileName);
    return classifier;
}

void trainAndPredict() {

    initAllData();

    Ptr<DTrees> rTree = Boost::create();
    rTree = train(rTree, allSamples);
    vector<float> results;
    try {

        results = {predict(rTree, testFromTrainingData),
                   predict(rTree, testHappy),
                   predict(rTree, testHappy2)};
    } catch (Exception &e) {
        cout << "";
    }
}

string formatter(vector<std::pair<std::string, vector<double>>> predictions_reg,
                 vector<std::pair<std::string, vector<double>>> predictions_class,
                 FaceAnalysis::FaceAnalyser face_analyser);


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
    string main_clnf_general = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/main_clnf_general.txt";
    string face_detector_location = concat(PATH, "/classifiers/haarcascade_frontalface_alt.xml");


    string au_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/AU_predictors/AU_all_best.txt";
    string tri_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/tris_68_full.txt";


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


    string result = formatter(predictions_reg, predictions_class, face_analyser);

    return prediction;
}

extern "C" {
JNIEXPORT jstring JNICALL
Java_ch_hepia_iti_opencvnativeandroidstudio_MainActivity_getEmoji(JNIEnv *env, jobject instance,
                                                                  jlong matAddrGray) {

    //trainAndPredict();

    ostringstream ss;
    ss << getAUs(matAddrGray);
    return env->NewStringUTF(ss.str().c_str());

}
}

string formatter(vector<std::pair<std::string, vector<double>>> predictions_reg,
                 vector<std::pair<std::string, vector<double>>> predictions_class,
                 FaceAnalysis::FaceAnalyser face_analyser) {
    stringstream ss;

    int num_class = predictions_class.size();
    int num_reg = predictions_reg.size();

    // Extract the indices of writing out first
    vector<string> au_reg_names = face_analyser.GetAURegNames();
    sort(au_reg_names.begin(), au_reg_names.end());
    vector<int> inds_reg;

    // write out ar the correct index
    for (string au_name : au_reg_names) {
        for (int i = 0; i < num_reg; ++i) {
            if (au_name.compare(predictions_reg[i].first) == 0) {
                inds_reg.push_back(i);
                break;
            }
        }
    }
    cout << "2***********************************************************************************"
         << endl;

    vector<string> au_class_names = face_analyser.GetAUClassNames();
    sort(au_class_names.begin(), au_class_names.end());
    vector<int> inds_class;

    // write out ar the correct index
    for (string au_name : au_class_names) {
        for (int i = 0; i < num_class; ++i) {
            if (au_name.compare(predictions_class[i].first) == 0) {
                inds_class.push_back(i);
                break;
            }
        }
    }

    // Read the header and find all _r and _c parts in a file and use their indices
    vector<string> tokens;

    int begin_ind = -1;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].find("AU") != string::npos && begin_ind == -1) {
            begin_ind = i;
            break;
        }
    }
    int end_ind = begin_ind + num_class + num_reg;


    int noOfAUs = 35;

    for (int i = 1; i < noOfAUs / 2; ++i) {
        for (int t = 1; t < noOfAUs / 2; ++t) {
            if (t >= begin_ind && t < end_ind) {
                if (t - begin_ind < num_reg) {
                    ss << "reg_" << (t - begin_ind) << ": "
                       << predictions_reg[inds_reg[t - begin_ind]].second[i - 1] << endl;
                } else {
                    ss << "class_" << (t - begin_ind - num_reg) << ": "
                       << predictions_class[inds_class[t - begin_ind - num_reg]].second[i - 1]
                       << endl;
                }
            }
            // else
            // {
            // 	cout << ", " << tokens[t];
            // }
        }
        ss << endl;
    }

    return ss.str();
}

extern "C"
{
JNIEXPORT jstring JNICALL
Java_ch_hepia_iti_opencvnativeandroidstudio_Open_foo(JNIEnv *env, jobject instance) {

    Ptr<DTrees> dtree = DTrees::create();
    dtree->setMaxDepth(10);
    dtree->setMinSampleCount(2);
    dtree->setRegressionAccuracy(0);
    dtree->setUseSurrogates(false);
    dtree->setMaxCategories(16);
    dtree->setCVFolds(0);
    dtree->setUse1SERule(false);
    dtree->setTruncatePrunedTree(false);
    dtree->setPriors(Mat());

    String filename = "data/data/ch.hepia.iti.opencvnativeandroidstudio/trainingData/emotions.csv";
    Ptr<TrainData> data = TrainData::loadFromCSV(filename, 0);
    data->getLayout();
    dtree->train(data);
    printf("OpenCV version %s (%d.%d.%d)\n",
           CV_VERSION,
           CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);

    String filenametest = "data/data/ch.hepia.iti.opencvnativeandroidstudio/trainingData/happy3.csv";
    Ptr<TrainData> dataTest = TrainData::loadFromCSV(filenametest, 0);


    float result = dtree->predict(dataTest->getSamples());
    stringstream sstream;
    sstream << "Success " << result;
    return env->NewStringUTF(sstream.str().c_str());

}
}

extern "C"
{
void JNICALL
Java_ch_hepia_iti_opencvnativeandroidstudio_MainActivity_salt(JNIEnv *env, jobject instance,
                                                              jlong matAddrGray,
                                                              jint nbrElem) {
    Mat &mGr = *(Mat *) matAddrGray;
//    for (int k = 0; k < nbrElem; k++) {
//        int i = rand() % mGr.cols;
//        int j = rand() % mGr.rows;
//        mGr.at<uchar>(j, i) = 255;
//    }
    vector<string> arguments;

    LandmarkDetector::FaceModelParameters det_parameters;
    LandmarkDetector::CLNF clnf_model;

    det_parameters.init();
    det_parameters.model_location = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/main_clnf_general.txt";
    det_parameters.face_detector_location = "data/data/ch.hepia.iti.opencvnativeandroidstudio/classifiers/haarcascade_frontalface_alt.xml";


    clnf_model.model_location_clnf = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/main_clnf_general.txt";
    clnf_model.face_detector_location_clnf = "data/data/ch.hepia.iti.opencvnativeandroidstudio/classifiers/haarcascade_frontalface_alt.xml";
    clnf_model.inits();

    __android_log_write(ANDROID_LOG_INFO, "JNIDATA", "Init Successfull!!");


    cv::Mat_<float> depth_image;
    cv::Mat_<uchar> grayscale_image;


    grayscale_image = mGr.clone();


    // The actual facial landmark detection / tracking
    bool detection_success = LandmarkDetector::DetectLandmarksInImage(grayscale_image, depth_image,
                                                                      clnf_model, det_parameters);

    if (detection_success) {
        LandmarkDetector::Draw(mGr, clnf_model);
    }

    stringstream sstream;
    sstream << "Value is " << detection_success << endl;
    //LOGD(sstream.str().c_str());
    __android_log_write(ANDROID_LOG_INFO, "JNIDATA", sstream.str().c_str());

    // The modules that are being used for tracking
    //LandmarkDetector::CLNF clnf_model;

    //det_parameters.init();
    //LandmarkDetector::CLNF clnf_model(det_parameters.model_location);

    //LandmarkDetector::DetectLandmarksInImage(grayscale_image, clnf_model, det_parameters);
//        ifstream infile();
//
//    string line;
//    ifstream myfile("/data/data/ch.hepia.iti.opencvnativeandroidstudio/model/model_eye/clnf_left_synth.txt");
//    sstream.str("");
//    if (myfile.is_open())
//    {
//        while ( getline (myfile,line) )
//        {
//            sstream << line << '\n';
//        }
//        myfile.close();
//    }
//
//    else sstream << "Unable to open file";
//
//    __android_log_write(ANDROID_LOG_INFO, "JNIDATA", sstream.str().c_str());


}
}

void initAllSamples() {
    allSamples = {
            {0.410933, 0.39483, 3.22378,    0.338012, 0.40643,     1.49396,    0.493247, -0.125391, -0.36427,   1.12024,    0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, -2.50168,   0.338012, 0.330743,    -0.416294,  0.493247, 0.112535,  0.779947,   1.42247,    0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, -1.1583,    0.338012, 0.74171,     0.941366,   0.493247, 1.12709,   -0.0490878, 0.961765,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, 0.637205,   0.338012, 2.18092,     2.28289,    0.493247, 2.11026,   0.542055,   1.10946,    0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, 1.74978,    0.338012, 1.34671,     2.46181,    0.493247, 1.45197,   -0.31531,   -0.0953508, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, 3.4129,     0.338012, 1.37541,     2.17323,    0.493247, 1.42742,   0.188008,   0.669802,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, -1.72799,   0.338012, 0.519962,    0.284628,   0.493247, -0.570612, -0.323785,  -0.41892,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
            {0.410933, 0.39483, -0.644556,  0.338012, -0.00396261, 0.810891,   0.493247, -0.986521, -0.538827,  0.00572037, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
            {0.410933, 0.39483, -1.1519,    0.338012, -0.00368801, 0.300969,   0.493247, -1.41105,  -0.468906,  0.301558,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
            {0.410933, 0.39483, -0.875947,  0.338012, 0.965427,    1.89435,    0.493247, 0.0170946, 1.76937,    0.703147,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, -1.09291,   0.338012, 2.14627,     2.23894,    0.493247, 0.790758,  3.58058,    0.449637,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
            {0.410933, 0.39483, -1.21545,   0.338012, 2.30475,     1.53966,    0.493247, 1.22476,   3.89454,    0.218981,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
            {0.410933, 0.39483, -0.28376,   0.338012, 0.29978,     0.444429,   0.493247, -0.943913, 0.0922714,  -0.257487,  0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0.410933, 0.39483, -0.0634723, 0.338012, -0.43969,    -0.0686456, 0.493247, -0.622837, 0.254162,   0.776314,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, -0.525441,  0.338012, -0.0131235,  -0.0758604, 0.493247, -0.391211, 0.173861,   0.482653,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, 0.263136,   0.338012, 0.694583,    1.33617,    0.493247, 0.771077,  0.674101,   -1.41127,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0},
            {0.410933, 0.39483, 0.260817,   0.338012, 0.701528,    1.13569,    0.493247, -0.73648,  -0.289481,  0.790368,   0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0},
            {0.410933, 0.39483, -0.958556,  0.338012, 0.375496,    0.825136,   0.493247, -0.931186, -0.334404,  -0.0454756, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0}
    };
}

void initTests() {
    testFromTrainingData = {0.410933, 0.39483, -0.958556, 0.338012, 0.375496,
                            0.825136, 0.493247,
                            -0.931186, -0.334404, -0.0454756, 0.20112, 0.409727,
                            0.378909, 0.563469,
                            0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 1, 0,
                            0, 1, 0};

    testHappy = {
            0.410933, 0.39483, -0.958556, 0.338012, 0.375496, 0.825136, 0.493247, -0.931186,
            -0.334404, -0.0454756, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755,
            0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0

    };

    testHappy2 = {
            0.410933, 0.39483, -1.78038, 0.338012, 1.0528, 1.05572, 0.493247, -0.538045, 2.23443,
            1.25076, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0,
            1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0
    };
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