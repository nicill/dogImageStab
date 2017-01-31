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

    // TODO temporary override until an algorithm has been found.
    double sumOfDistances = 0;
    for (DMatch match : goodMatches) {
        sumOfDistances += match.distance;
    }
    double avgdistance = sumOfDistances / goodMatches.size();

    std::cout << "Average distance: " << avgdistance <<" (NO SIMILARITY CALCULATION DONE!)" << std::endl;
    return 1;
}

/**
 * Uses the class featureDetector and descriptorMatcher to compute matches between the two given images.
 * @param img1 The first image.
 * @param img2 The second image.
 * @return Two matches for each point.
 */
vector<vector<DMatch>> featureComparer::getMatches(InputArray img1, InputArray img2) {
    vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    vector<vector<DMatch>> matchess;

    // Detect keypoints and compute descriptors (feature vectors)
    this->featureDetector->detect(img1, keypoints_1);
    this->featureDetector->detect(img2, keypoints_2);

    this->featureDetector->compute(img1, keypoints_1, descriptors_1);
    this->featureDetector->compute(img2, keypoints_2, descriptors_2);

    // Match descriptor vectors
    this->descriptorMatcher->knnMatch(
            descriptors_1,
            descriptors_2,
            matchess,
            2);

    return matchess;
}
