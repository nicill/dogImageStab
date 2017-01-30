//
// Created by tokuyama on 17/01/30.
//

#ifndef FEATURECOMPARER_FEATURECOMPARER_H_
#define FEATURECOMPARER_FEATURECOMPARER_H_

#include "../framewiseSimilarityMetric.h"
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/opencv.hpp>

using std::vector;
using cv::DescriptorMatcher;
using cv::DMatch;

class featureComparer : public framewiseSimilarityMetric {
public:
    enum featureDetectorType { SIFT, SURF, ORB };
    enum descriptorMatcherType { BF };  // FLANN?

    double computeSimilarity(Mat* im1, Mat* im2);

    featureComparer(
            featureDetectorType givenDetectorType = SIFT,
            descriptorMatcherType givenMatcherType = BF);

private:
    featureDetectorType detectorType;
    descriptorMatcherType matcherType;

    Ptr<Feature2D> featureDetector;
    DescriptorMatcher *descriptorMatcher;

    vector<vector<DMatch>> getMatches(InputArray img1, InputArray img2);
};


#endif  // FEATURECOMPARER_FEATURECOMPARER_H_
