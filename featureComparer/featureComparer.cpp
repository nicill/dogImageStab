//
// Created by tokuyama on 17/01/30.
//

#include "featureComparer.h"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;

featureComparer::featureComparer(featureDetector featureDetector, descriptorMatcher descriptorMatcher)
{
    this->detectorType = featureDetector;
    this->matcherType = descriptorMatcher;

    switch (featureDetector)
    {
        case featureComparer::SIFT:
            this->featureDetector = xfeatures2d::SIFT::create();
            break;
        case featureComparer::SURF:
            this->featureDetector = xfeatures2d::SURF::create();
            break;
        case featureComparer::ORB:
            this->featureDetector = ORB::create();
            break;
        default:
            throw("This descriptor hasn't been implemented yet.");
    }

    switch (descriptorMatcher)
    {
        case featureComparer::BF:
            BFMatcher matcher;
            this->descriptorMatcher = matcher;
        default:
            throw("This matcher hasn't been implemented yet.");
    }
}

double featureComparer::computeSimilarity(Mat* im1, Mat* im2)
{
    vector<vector<DMatch>> matchess = getMatches(*im1, *im2);

    // Implmentation is based on the assumption of only two matches being selected.
    assert(matchess[0].size() == 2);

    // Filter matches based on metric proposed by Lowe (2004), p. 104.
    vector<DMatch> goodMatches;
    for(vector<DMatch> matches : matchess)
    {
        if (matches[0].distance < 0.8 * matches[1].distance)
        {
            goodMatches.push_back(matches[0]);
        }
    }

    // TODO Calculate similarity score in [0,1].
    assert_perror(1);
    return 0;
}

// Implementation is based on: http://stackoverflow.com/a/27533437
vector<vector<DMatch>> featureComparer::getMatches(InputArray img1, InputArray img2)
{
    // Detect keypoints
    vector<KeyPoint> keypoints_1, keypoints_2;
    this->featureDetector->detect(img1, keypoints_1);
    this->featureDetector->detect(img2, keypoints_2);

    // Calculate descriptors (feature vectors)
    Mat descriptors_1, descriptors_2;
    this->featureDetector->compute(img1, keypoints_1, descriptors_1);
    this->featureDetector->compute(img2, keypoints_2, descriptors_2);

    // Match descriptor vectors
    vector<vector<DMatch>> matchess;
    this->descriptorMatcher.knnMatch(
            (InputArray)descriptors_1,
            (InputArray)descriptors_2,
            matchess,
            2);

    return matchess;
}
