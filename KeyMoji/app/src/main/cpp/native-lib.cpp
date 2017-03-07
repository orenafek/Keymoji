#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "openface/LandmarkDetector/include/LandmarkCoreIncludes.h"
#include "opencv2/ml/ml.hpp"
#include <android/log.h>
#include <vector>



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

string concat(string s1, string s2){
    stringstream ss;
    ss << s1 << s2;
    return ss.str();
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
    string face_detector_location = concat(PATH,"/classifiers/haarcascade_frontalface_alt.xml");

    LandmarkDetector::CLNF face_model(main_clnf_general);
    face_model.face_detector_HAAR.load(face_detector_location);


    string au_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/AU_predictors/AU_all_best.txt";
    string tri_loc = "data/data/ch.hepia.iti.opencvnativeandroidstudio/model/tris_68_full.txt";


    auto v = vector<cv::Vec3d>();
    FaceAnalysis::FaceAnalyser face_analyser(v, 0.7, 112, 112, au_loc, tri_loc);
    Mat grayscale_image = captured_image.clone();
    LandmarkDetector::FaceModelParameters det_parameters;
    det_parameters.init();
    det_parameters.model_location = main_clnf_general;
    det_parameters.face_detector_location = face_detector_location;


    LandmarkDetector::DetectLandmarksInImage(grayscale_image, face_model, det_parameters);
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


    string result = formatter(predictions_reg, predictions_class, face_analyser);
    return env->NewStringUTF(result.c_str());

}
}

string formatter(vector<std::pair<std::string, vector<double>>> predictions_reg,
               vector<std::pair<std::string, vector<double>>> predictions_class,
               FaceAnalysis::FaceAnalyser face_analyser){
    stringstream ss;

    int num_class = predictions_class.size();
    int num_reg = predictions_reg.size();

    // Extract the indices of writing out first
    vector<string> au_reg_names = face_analyser.GetAURegNames();
    sort(au_reg_names.begin(), au_reg_names.end());
    vector<int> inds_reg;

    // write out ar the correct index
    for (string au_name : au_reg_names)
    {
        for (int i = 0; i < num_reg; ++i)
        {
            if (au_name.compare(predictions_reg[i].first) == 0)
            {
                inds_reg.push_back(i);
                break;
            }
        }
    }
    cout << "2***********************************************************************************"<<endl;

    vector<string> au_class_names = face_analyser.GetAUClassNames();
    sort(au_class_names.begin(), au_class_names.end());
    vector<int> inds_class;

    // write out ar the correct index
    for (string au_name : au_class_names)
    {
        for (int i = 0; i < num_class; ++i)
        {
            if (au_name.compare(predictions_class[i].first) == 0)
            {
                inds_class.push_back(i);
                break;
            }
        }
    }
   
    // Read the header and find all _r and _c parts in a file and use their indices
    vector<string> tokens;
   
    int begin_ind = -1;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].find("AU") != string::npos && begin_ind == -1)
        {
            begin_ind = i;
            break;
        }
    }
    int end_ind = begin_ind + num_class + num_reg;

   
    int noOfAUs = 35;

    for (int i = 1; i < noOfAUs / 2; ++i)
    {
        for (int t = 1; t < noOfAUs / 2; ++t)
        {
            if (t >= begin_ind && t < end_ind)
            {
                if(t - begin_ind < num_reg)
                {
                    ss << "reg_" << (t - begin_ind)<< ": " << predictions_reg[inds_reg[t - begin_ind]].second[i - 1] << endl;
                }
                else
                {
                    ss << "class_" << (t - begin_ind - num_reg) << ": " << predictions_class[inds_class[t - begin_ind - num_reg]].second[i - 1] << endl;
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
