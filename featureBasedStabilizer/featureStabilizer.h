//
// Created by yago on 17/02/20.
//

#ifndef DOGIMAGESTABILIZATION_FEATURESTABILIZER_H
#define DOGIMAGESTABILIZATION_FEATURESTABILIZER_H

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"


using std::string;
using std::vector;
using cv::DescriptorMatcher;
using cv::DMatch;
using namespace cv;


class featureStabilizer {

public:
    enum type { SIFT_BFL2 = 0, SURF_BFL2 = 1, ORB_BFHAMMING = 2, BRISK_BFHAMMING=3};
    enum featureDetectorType { SIFT = 0, SURF = 1, ORB = 2, BRISK=3 };
    enum descriptorMatcherType { BF_L2 = 0, BF_HAMMING = 1 }; // BF_HAMMING2, FLANN?, Hamming is a brute force matcher

    //double computeSimilarity(Mat* im1, Mat* im2);
    void activateVerbosity();

    featureStabilizer(type givenType = SIFT_BFL2);
    ~featureStabilizer();

    vector<vector<DMatch>> getMatches(Mat* img1, Mat* img2,vector<KeyPoint> * keypoints_1, vector<KeyPoint> * keypoints_2D);
    bool hasGoodMatch(vector<DMatch> possibleMatches);

private:
    featureDetectorType detectorType;
    descriptorMatcherType matcherType;

    // Possible feature detectors: http://docs.opencv.org/trunk/d0/d13/classcv_1_1Feature2D.html
    Ptr<Feature2D> featureDetector;
    // Possible descriptor matchers: http://docs.opencv.org/trunk/db/d39/classcv_1_1DescriptorMatcher.html
    Ptr<DescriptorMatcher> descriptorMatcher;
    // Usage example: https://github.com/kipr/opencv/blob/master/samples/cpp/descriptor_extractor_matcher.cpp
    // also check: http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_feature2d/py_matcher/py_matcher.html

    int processedComparisons;
    bool verbose = false;

    vector<DMatch>* goodMatches();
    void message(string text);

};


#endif //DOGIMAGESTABILIZATION_FEATURESTABILIZER_H




