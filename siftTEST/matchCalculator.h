//
// Created by tokuyama on 17/01/30.
//

#ifndef DOGIMAGESTABILIZATION_SIFTCALCULATOR_H
#define DOGIMAGESTABILIZATION_SIFTCALCULATOR_H

#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;

class matchCalculator {
public:
    enum featureDetector { SIFT, SURF, ORB };
    enum descriptorMatcher { BF }; // FLANN?

    static std::vector< cv::DMatch > getMatches(
            cv::InputArray img1,
            cv::InputArray img2,
            matchCalculator::featureDetector detector,
            matchCalculator::descriptorMatcher matcher);

private:
    static std::vector< cv::DMatch > getMatches(
            cv::InputArray img1,
            cv::InputArray img2,
            cv::Ptr< Feature2D > featureDetector,
            cv::DescriptorMatcher descriptorMatcher);
};


#endif //DOGIMAGESTABILIZATION_SIFTCALCULATOR_H
