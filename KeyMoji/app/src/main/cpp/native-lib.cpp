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

InputArray getPrediction() {

    vector<double> v = {0.410933, 0.39483, -0.644556, 0.338012, -0.00396261,
                        0.810891, 0.493247, -0.986521, -0.538827, 0.00572037,
                        0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755,
                        0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        1, 0};

    Mat m = Mat(1, v.size(), CV_32FC1);

    for (int i = 0; i < v.size(); ++i) {
        m.at<double>(0, i) = v[i];
    }
    return _InputArray(m);
}

Ptr<RTrees> giveMeTrainedTree() {
    vector<vector<double>> allSamples = {

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

    Mat m_samples = Mat(allSamples.size(), allSamples[0].size(), CV_32FC1);

    for (int i = 0; i < allSamples.size(); ++i) {
        for (int j = 0; j < allSamples[i].size(); ++j) {
            m_samples.at<double>(i, j) = allSamples[i][j];
        }
    }

    vector<int> all_responses = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6};
    Mat m_responses = Mat(allSamples.size(), 1, CV_32S);

    for (int i = 0; i < allSamples.size(); ++i) {
        m_responses.at<int>(i) = all_responses[i];
    }

    InputArray samples = _InputArray(m_samples);
    InputArray labels = _InputArray(m_responses);
    Ptr<RTrees> rTree = RTrees::create();
    //rTree->setMaxDepth(10);
    rTree->setMinSampleCount(2);
    //rTree->setRegressionAccuracy(0);
    rTree->setUseSurrogates(false);
    rTree->setMaxCategories(6);
    /*rTree->setPriors(Mat());
    rTree->setCalculateVarImportance(true);
    rTree->setActiveVarCount(0);
    rTree->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 0));*/
    try {
        rTree->train(samples, 0, labels);
    } catch (Exception &ex) {
        cout << "";
    }

    catch (...) {
        cout << "";
    }


    vector<double> testFromTrainingData = {0.410933, 0.39483, -0.958556, 0.338012, 0.375496,
                                           0.825136, 0.493247,
                                           -0.931186, -0.334404, -0.0454756, 0.20112, 0.409727,
                                           0.378909, 0.563469,
                                           0.543542, 0.480755, 0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 1, 0,
                                           0, 1, 0};

    vector<double> testHappy = {
            0.410933, 0.39483, -0.958556, 0.338012, 0.375496, 0.825136, 0.493247, -0.931186,
            -0.334404, -0.0454756, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755,
            0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0

    };

    vector<double> testHappy2 = {
            0.410933, 0.39483, -1.78038, 0.338012, 1.0528, 1.05572, 0.493247, -0.538045, 2.23443,
            1.25076, 0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755, 0.613564, 0, 0, 0,
            1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0
    };

    Mat testFromTrainingDataMat = Mat(1, testFromTrainingData.size(), CV_32FC1);
    Mat testHappyMat = Mat(1, testHappy.size(), CV_32FC1);
    Mat testHappyMat2 = Mat(1, testHappy2.size(), CV_32FC1);

    for (int i = 0; i < testFromTrainingData.size(); ++i) {
        testFromTrainingDataMat.at<double>(0, i) = testFromTrainingData[i];
        testHappyMat.at<double>(0, i) = testHappy[i];
        testHappyMat2.at<double>(0, i) = testHappy2[i];
    }
    InputArray testFromTrainingDataInputArray = _InputArray(testFromTrainingDataMat);
    InputArray testHappyInputArray = _InputArray(testHappyMat);
    InputArray testHappyInputArray2 = _InputArray(testHappyMat2);

    try {
        float testFromTrainingDataResult = rTree->predict(testFromTrainingDataInputArray);
        float testHappyResult = rTree->predict(testHappyInputArray);
        float testHappyResult2 = rTree->predict(testHappyInputArray2);
    } catch (Exception &e) {
        cout << "";
    }

    return rTree;

}

string formatter(vector<std::pair<std::string, vector<double>>> predictions_reg,
                 vector<std::pair<std::string, vector<double>>> predictions_class,
                 FaceAnalysis::FaceAnalyser face_analyser);

extern "C" {
JNIEXPORT jstring JNICALL
Java_ch_hepia_iti_opencvnativeandroidstudio_MainActivity_getEmoji(JNIEnv *env, jobject instance,
                                                                  jlong matAddrGray) {


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


//    cv::Mat sim_warped_img;
//    face_analyser.GetLatestAlignedFace(sim_warped_img);


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


    string result = formatter(predictions_reg, predictions_class, face_analyser);


    Ptr<RTrees> rtrees = RTrees::create();
    rtrees->setMaxDepth(10);
    rtrees->setMinSampleCount(2);
    rtrees->setRegressionAccuracy(0);
    rtrees->setUseSurrogates(false);
    rtrees->setMaxCategories(16);
    rtrees->setPriors(Mat());
    rtrees->setCalculateVarImportance(true);
    rtrees->setActiveVarCount(0);
    rtrees->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 0));

    string filename = "/data/data/ch.hepia.iti.opencvnativeandroidstudio/trainingData/TrainingData.csv";
    Ptr<TrainData> data = TrainData::loadFromCSV(filename, 0, 0, 1);

    bool flag = data.empty();

    rtrees->train(data);
    Mat tmp = data->getResponses();
    String matAsString(tmp.begin<char>(), tmp.end<char>());
    double x = tmp.at<double>(0, 0);
    double y = tmp.at<double>(0, 1);


    Mat tmp2 = data->getTrainResponses();
    String matAsString2(tmp2.begin<char>(), tmp2.end<char>());
    double x2 = tmp2.at<double>(0, 0);

    Ptr<RTrees> rTrees = giveMeTrainedTree();

    bool b = rTrees->isTrained();

    //InputArray inputArray = getPrediction();
    vector<double> v2 = {0.410933, 0.39483, -0.644556, 0.338012, -0.00396261,
                         0.810891, 0.493247, -0.986521, -0.538827, 0.00572037,
                         0.20112, 0.409727, 0.378909, 0.563469, 0.543542, 0.480755,
                         0.613564, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         1, 0};

    Mat m = Mat(1, v2.size(), CV_32FC1);

    for (int i = 0; i < v2.size(); ++i) {
        m.at<double>(0, i) = v2[i];
    }
    InputArray inputArray = _InputArray(m);
    OutputArray outputArray = _OutputArray(Mat(1, 1, CV_32S));
    try {
        float f = rTrees->predict(inputArray);
    } catch (Exception &e) {
        cout << "";
    }
    stringstream ss;
    //ss << outputArray.getMat();

    string prediction = ss.str();
    return env->NewStringUTF(result.c_str());

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
