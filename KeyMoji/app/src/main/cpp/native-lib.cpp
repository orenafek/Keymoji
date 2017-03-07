#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "openface/LandmarkDetector/include/LandmarkCoreIncludes.h"
#include "opencv2/ml/ml.hpp"
#include <android/log.h>
#include <vector>
#include <unordered_pair.h>


#include "openface/FaceAnalyser/include/FaceAnalyser.h"
#include <iostream>
#include <fstream>
#define  LOG_TAG    "ndk-tag"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
using namespace std;
using namespace cv;
using namespace cv::ml;
#define PACKAGE ch.hepia.iti.opencvnativeandroidstudio
#define PATH "data/data/ch.hepia.itsi.opencvnativeandroidstudio"

extern "C" {
JNIEXPORT jint JNICALL
Java_ch_hepia_iti_opencvnativeandroidstudio_MainActivity_getEmoji(JNIEnv *env, jobject instance,
                                                                  jlong matAddrGray) {


    Mat &captured_image = *(Mat *) matAddrGray;
    string main_clnf_general = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/main_clnf_general.txt";
    LandmarkDetector::CLNF face_model(main_clnf_general);


    string au_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/AU_predictors/AU_all_best.txt";
    string tri_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/tris_68_full.txt";

    auto v = vector<cv::Vec3d>();
    FaceAnalysis::FaceAnalyser face_analyser(v, 0.7, 112, 112, au_loc, tri_loc);
    int time_stamp = 4;
    face_analyser.AddNextFrame(captured_image, face_model, time_stamp, false, true);// last parameter is quiet mode inverted !det_parameters.quiet_mode


//    cv::Mat sim_warped_img;
//    face_analyser.GetLatestAlignedFace(sim_warped_img);


    bool dynamic = true;
    vector<double> certainties;
    vector<bool> successes;
    vector<double> timestamps;
    vector<std::pair<std::string, vector<double>>> predictions_reg;
    vector<std::pair<std::string, vector<double>>> predictions_class;
    face_analyser.ExtractAllPredictionsOfflineReg(predictions_reg, certainties, successes, timestamps, dynamic);
    face_analyser.ExtractAllPredictionsOfflineClass(predictions_class, certainties, successes, timestamps, dynamic);

    return 0;

}
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
    printf ("OpenCV version %s (%d.%d.%d)\n",
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
void JNICALL Java_ch_hepia_iti_opencvnativeandroidstudio_MainActivity_salt(JNIEnv *env, jobject instance,
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

    __android_log_write(ANDROID_LOG_INFO, "JNIDATA",  "Init Successfull!!");


    cv::Mat_<float> depth_image;
    cv::Mat_<uchar> grayscale_image;


    grayscale_image = mGr.clone();


    // The actual facial landmark detection / tracking
    bool detection_success = LandmarkDetector::DetectLandmarksInImage(grayscale_image, depth_image, clnf_model, det_parameters);

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
