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
    enum featureDetectorType { SIFT = 0, SURF = 1 }; // SURF? ORB?
    enum descriptorMatcherType { BF_L2 = 0 };  // FLANN?

    double computeSimilarity(Mat* im1, Mat* im2);
    void activateVerbosity();

    featureComparer(
            featureDetectorType givenDetectorType = SIFT,
            descriptorMatcherType givenMatcherType = BF_L2);
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
