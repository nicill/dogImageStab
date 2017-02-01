//
// Created by tokuyama on 17/01/30.
//

#include "featureComparer.h"

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

    vector<vector<DMatch>> matchess = this->getMatches(im1, im2);

    // Implmentation assumes only two matches being selected.
    if (matchess.size() != 0) assert(matchess[0].size() == 2);

    // Filter matches based on metric proposed by Lowe (2004), p. 104.
    // Alternatively use crosscheck? http://docs.opencv.org/3.0-beta/doc/py_tutorials/py_feature2d/py_matcher/py_matcher.html
    vector<DMatch> goodMatches;
    for (vector<DMatch> matches : matchess) {
        // Matches are sorted from smallest distance to biggest.
        if (matches[0].distance < 0.8 * matches[1].distance) {
            goodMatches.push_back(matches[0]);
        }
    }

    // No matches found
    if (goodMatches.size() == 0) return 0;

    // TODO temporary override until an algorithm has been found.
    double sumOfDistances = 0;
    double mindistance = goodMatches[0].distance, maxdistance = goodMatches[0].distance;
    for (DMatch match : goodMatches) {
        sumOfDistances += match.distance;

        if (match.distance < mindistance) mindistance = match.distance;
        else if (match.distance > maxdistance) maxdistance = match.distance;
    }
    double avgdistance = sumOfDistances / goodMatches.size();

    // TODO temporary output
    std::ofstream outfile;
    outfile.open("results.txt", std::ios_base::app);
    outfile << this->processedComparisons << " | AVG: " << avgdistance << ", MIN: " << mindistance << ", MAX: " << maxdistance << std::endl;
    outfile.close();

    // std::cout << "Average distance: " << avgdistance << std::endl;
    return 1;
}

/**
 * Computes matches between the two given images.
 * @param img1 The first image.
 * @param img2 The second image.
 * @return A list of the best two matches for each determined keypoint.
 */
vector<vector<DMatch>> featureComparer::getMatches(Mat* img1, Mat* img2) {
    vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    vector<vector<DMatch>> matchess;

    // Detect keypoints and compute descriptors (feature vectors)
    this->featureDetector->detectAndCompute(*img1, noArray(), keypoints_1, descriptors_1);
    this->featureDetector->detectAndCompute(*img2, noArray(), keypoints_2, descriptors_2);

    // Match descriptors and retrieve the k=2 best results
    this->descriptorMatcher->knnMatch(
            descriptors_1,
            descriptors_2,
            matchess,
            2);

    // TODO seems shady that we need to check this
    bool matchessInitialised = 1 < matchess.size();
    bool matchess_0_Initialised = (matchessInitialised && 1 < matchess[0].size());

    if (!matchessInitialised || !matchess_0_Initialised) {
        vector<vector<DMatch>> *temp = new vector<vector<DMatch>>();
        matchess = *temp;
    }

    return matchess;
}
