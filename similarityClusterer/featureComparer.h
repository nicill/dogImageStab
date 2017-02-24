//
// Created by tokuyama on 17/01/30.
//

#ifndef DOGIMAGESTABILIZATION_FEATURECOMPARER_H
#define DOGIMAGESTABILIZATION_FEATURECOMPARER_H

#include "framewiseSimilarityMetric.h"
#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"

using std::string;
using std::vector;
using cv::DescriptorMatcher;
using cv::DMatch;

class featureComparer : public framewiseSimilarityMetric {
public:
    enum type { SIFT_BFL2 = 0, SURF_BFL2 = 1, ORB_BFHAMMING = 2,BRISK_BFHAMMING=3 };
    enum featureDetectorType { SIFT = 0, SURF = 1, ORB = 2, BRISK=3  };
    enum descriptorMatcherType { BF_L2 = 0, BF_HAMMING = 1 }; // BF_HAMMING2, FLANN?

    double computeSimilarity(Mat* im1, Mat* im2);
    void activateVerbosity();

    featureComparer(type givenType = SIFT_BFL2);
    ~featureComparer();

private:
    featureDetectorType detectorType;
    descriptorMatcherType matcherType;

    // Possible feature detectors: http://docs.opencv.org/trunk/d0/d13/classcv_1_1Feature2D.html
    Ptr<Feature2D> featureDetector;
    // Possible descriptor matchers: http://docs.opencv.org/trunk/db/d39/classcv_1_1DescriptorMatcher.html
    Ptr<DescriptorMatcher> descriptorMatcher;

    int processedComparisons;
    bool verbose = false;

    vector<vector<DMatch>> getMatches(Mat* img1, Mat* img2);
    bool hasGoodMatch(vector<DMatch> possibleMatches);
    void message(string text);
};

#endif  // FEATURECOMPARER_FEATURECOMPARER_H
