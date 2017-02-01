//
// Created by tokuyama on 17/01/30.
//

#include "featureComparer.h"

// TODO debug variables
//                #    avg     min     max
vector<std::tuple<int, double, double, double>> distancesPerFrame;

/**
 * Constructor.
 * @param givenDetectorType Type of detector.
 * @param givenMatcherType Type of matcher.
 * For SIFT, SURF etc. use L2. For ORB, BRIEF, BRISK etc. use HAMMING.
 * If ORB is using WTA_K == 3 or 4 use HAMMING2.
 */
featureComparer::featureComparer(
        featureDetectorType givenDetectorType,
        descriptorMatcherType givenMatcherType) {
    this->detectorType = givenDetectorType;
    this->matcherType = givenMatcherType;
    this->processedComparisons = 0;

    switch (givenDetectorType) {
        case featureComparer::SIFT:
            this->featureDetector = xfeatures2d::SIFT::create();
            break;
        default:
            throw("This descriptor hasn't been implemented yet.");
    }

    switch (givenMatcherType) {
        case featureComparer::BF_L2:
            this->descriptorMatcher = BFMatcher::create();
            break;
            // TODO BF_HAMMING, BF_HAMMING2, FLANN
        default:
            throw("This matcher hasn't been implemented yet.");
    }
}

double featureComparer::computeSimilarity(Mat* im1, Mat* im2) {
    this->processedComparisons++;

    vector<vector<DMatch>> matchesOfAllKeypoints = this->getMatches(im1, im2);

    vector<DMatch> goodMatches;
    for (vector<DMatch> matchesAtKeypoint : matchesOfAllKeypoints) {
        // Match list might not contain any matches
        if (matchesAtKeypoint.size() == 0) {
            continue;
        }

        // Matches are sorted from best to worst.
        if (this->hasGoodMatch(matchesAtKeypoint)) {
            goodMatches.push_back(matchesAtKeypoint[0]);
        }
    }

    // No matches found
    if (goodMatches.size() == 0) return 0;

    // TODO temporary override until an algorithm has been found.
    double sumOfDistances = 0;
    double average = 0;
    double minimum = goodMatches[0].distance;
    double maximum = goodMatches[0].distance;

    for (DMatch match : goodMatches) {
        sumOfDistances += match.distance;

        if (match.distance < minimum) minimum = match.distance;
        if (match.distance > maximum) maximum = match.distance;
    }
    average = sumOfDistances / goodMatches.size();

    //                                     #    avg     min     max
    distancesPerFrame.push_back(std::tuple<int, double, double, double>(
            this->processedComparisons, average, minimum, maximum));

    return (double)goodMatches.size() / matchesOfAllKeypoints.size();
}

/**
 * Computes matches between the two given images.
 * @param img1 The first image.
 * @param img2 The second image.
 * @return A list of the best two matches for each determined keypoint.
 * Elements might contain no matches!
 */
vector<vector<DMatch>> featureComparer::getMatches(Mat* img1, Mat* img2) {
    vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    vector<vector<DMatch>> matchesOfAllKeypoints;

    // Detect keypoints and compute descriptors (feature vectors)
    this->featureDetector->detectAndCompute(*img1, noArray(), keypoints_1, descriptors_1);
    this->featureDetector->detectAndCompute(*img2, noArray(), keypoints_2, descriptors_2);

    // Match descriptors and retrieve the k=2 best results
    this->descriptorMatcher->knnMatch(
            descriptors_1,
            descriptors_2,
            matchesOfAllKeypoints,
            2);

    return matchesOfAllKeypoints;
}

/**
 * Checks the given list of matches if the best one can be considered "good".
 * Currently only works with SIFT!
 * @param possibleMatches All possible matches for a keypoint. Must be of size 2!
 */
bool featureComparer::hasGoodMatch(vector<DMatch> possibleMatches) {
    // Filter matches based on metric proposed by Lowe (2004), p. 104.
    // TODO WARNING: Only to be used with SIFT!
    // Alternatively use crosscheck? http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_feature2d/py_matcher/py_matcher.html
    assert(possibleMatches.size() <= 2);

    if (possibleMatches.size() != 2) return false;
    else return possibleMatches[0].distance < 0.8 * possibleMatches[1].distance;
}

/**
 * Destructor. Used for debugging purposes.
 */
featureComparer::~featureComparer() {
    if (1 < distancesPerFrame.size()) return;

    double sumOfAverages = 0;
    double globalAverage = std::get<1>(distancesPerFrame[0]);
    double globalMinimum = std::get<2>(distancesPerFrame[0]);
    double globalMaximum = std::get<3>(distancesPerFrame[0]);

    // Compute total maximum, minimum and average
    //              #    avg     min     max
    for (std::tuple<int, double, double, double> entry : distancesPerFrame) {
        sumOfAverages += std::get<1>(entry);
        if (std::get<2>(entry) < globalMinimum) {}
        if (std::get<3>(entry) > globalMaximum) {}

        std::cout << std::get<0>(entry) << " | " << "AVG: " << std::get<1>(entry)
                  << ", MIN: " << std::get<2>(entry) << ", MAX: " << std::get<3>(entry)
                  << std::endl;
    }
    globalAverage = sumOfAverages / distancesPerFrame.size();

    std::cout << std::endl << std::endl << "GLOBAL" << std::endl << "----------" << std::endl
              << "AVG: " << globalAverage << ", MIN: " << globalMinimum
              << ", MAX: " << globalMaximum << std::endl;
}
