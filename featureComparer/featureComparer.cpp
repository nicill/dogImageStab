//
// Created by tokuyama on 17/01/30.
//

#include "featureComparer.h"

featureComparer::featureComparer(
        featureDetectorType givenDetectorType,
        descriptorMatcherType givenMatcherType) {
    this->detectorType = givenDetectorType;
    this->matcherType = givenMatcherType;

    switch (givenDetectorType) {
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

    switch (givenMatcherType) {
        case featureComparer::BF:
            this->descriptorMatcher = BFMatcher::create();
            break;
            // TODO FLANN
        default:
            throw("This matcher hasn't been implemented yet.");
    }
}

double featureComparer::computeSimilarity(Mat* im1, Mat* im2) {
    vector<vector<DMatch>> matchess = getMatches(*im1, *im2);

    // Implmentation assumes only two matches being selected.
    assert(matchess[0].size() == 2);

    // Filter matches based on metric proposed by Lowe (2004), p. 104.
    vector<DMatch> goodMatches;
    for (vector<DMatch> matches : matchess) {
        if (matches[0].distance < 0.8 * matches[1].distance) {
            goodMatches.push_back(matches[0]);
        }
    }

    // No matches found
    if (goodMatches.size() == 0) return 0;

    // TODO proposal: Compare number of matches with distance < 100 to those with bigger distances.
    int matches0_100 = 0;
    int matches100_ = 0;
    double relation;

    for (DMatch match : goodMatches) {
        if (match.distance < 100) matches0_100++;
        else matches100_++;
    }

    // Make sure the calculation works.
    if (matches0_100 == 0 || matches100_ == 0) {
        matches0_100++;
        matches100_++;
    }

    relation = matches0_100 / matches100_;

    // TODO debug
    // std::cout << "matches0_100: " << matches0_100 <<", matches100_: " << matches100_ << ", relation: " << relation << std::endl;
    int maxVal = 2000;

    if (relation > maxVal) return 1;
    // relation is now between 0 and 2000
    else return relation / maxVal;
}

// Implementation is based on: http://stackoverflow.com/a/27533437
vector<vector<DMatch>> featureComparer::getMatches(
        InputArray img1, InputArray img2) {
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

    this->descriptorMatcher->knnMatch(
            (InputArray)descriptors_1,
            (InputArray)descriptors_2,
            matchess,
            2);

    return matchess;
}
