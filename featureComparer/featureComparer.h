//
// Created by tokuyama on 17/01/30.
//

#ifndef DOGIMAGESTABILIZATION_SIFTCALCULATOR_H
#define DOGIMAGESTABILIZATION_SIFTCALCULATOR_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "../framewiseSimilarityMetric.h"

using namespace std;
using namespace cv;

class featureComparer : public framewiseSimilarityMetric
{
public:
    enum featureDetector { SIFT, SURF, ORB };
    enum descriptorMatcher { BF }; // FLANN?

    double computeSimilarity(Mat* im1, Mat* im2);
    featureComparer(featureDetector = SIFT, descriptorMatcher = BF);

private:
    featureDetector detectorType;
    descriptorMatcher matcherType;

    Ptr<Feature2D> featureDetector;
    DescriptorMatcher descriptorMatcher;

    vector<vector<DMatch>> getMatches(InputArray img1, InputArray img2);
};


#endif //DOGIMAGESTABILIZATION_SIFTCALCULATOR_H
