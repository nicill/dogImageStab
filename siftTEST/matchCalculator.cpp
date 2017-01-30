//
// Created by tokuyama on 17/01/30.
//

#include "matchCalculator.h"
#include "opencv2/xfeatures2d.hpp"

using namespace std;
using namespace cv;

static std::vector< cv::DMatch > matchCalculator::getMatches(
        cv::InputArray img1,
        cv::InputArray img2,
        matchCalculator::featureDetector detectorType,
        matchCalculator::descriptorMatcher matcherType)
{
    cv::Ptr< Feature2D > featureDetector;

    switch (detectorType)
    {
        case matchCalculator::SIFT:
            featureDetector = xfeatures2d::SIFT::create();
            break;
        case matchCalculator::SURF:
            featureDetector = xfeatures2d::SURF::create();
            break;
        default:
            cerr << "NOT IMPLEMENTED!" << endl << "This descriptor hasn't been implemented yet. Using ORB." << endl;
            // fall-through intended
        case matchCalculator::ORB:
            featureDetector = ORB::create();
            break;
    }

    switch (matcherType)
    {
        default:
            cerr << "NOT IMPLEMENTED!" << endl << "This matcher hasn't been implemented yet. Using BF." << endl;
            // fall-through intended
        case matchCalculator::BF:
            BFMatcher descriptorMatcher;
            return getMatches(img1, img2, featureDetector, descriptorMatcher);
    }
}

static std::vector< cv::DMatch > matchCalculator::getMatches(
        cv::InputArray img1,
        cv::InputArray img2,
        cv::Ptr< Feature2D > featureDetector,
        cv::DescriptorMatcher descriptorMatcher)
{
    // Detect keypoints
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    featureDetector->detect( img1, keypoints_1 );
    featureDetector->detect( img2, keypoints_2 );

    // Calculate descriptors (feature vectors)
    Mat descriptors_1, descriptors_2;
    featureDetector->compute( img1, keypoints_1, descriptors_1 );
    featureDetector->compute( img2, keypoints_2, descriptors_2 );

    // Match descriptor vectors
    std::vector< DMatch > matches;
    descriptorMatcher.match( (InputArray)descriptors_1, (InputArray)descriptors_2, matches );

    return matches;
}
