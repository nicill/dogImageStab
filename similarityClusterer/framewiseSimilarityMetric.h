//
// Created by yago on 17/01/23.
//

#ifndef DOGIMAGESTABILIZATION_FRAMEWISESIMILARITYMETRIC_H
#define DOGIMAGESTABILIZATION_FRAMEWISESIMILARITYMETRIC_H

#include <opencv2/core/core.hpp>
using namespace cv;

class framewiseSimilarityMetric {
public:
    // pure virtual function
    virtual double computeSimilarity(Mat* im1, Mat* im2 ) = 0;
    virtual void activateVerbosity() = 0;
};



#endif //DOGIMAGESTABILIZATION_FRAMEWISESIMILARITYMETRIC_H


// ideas for similarity metrics:
// NICE opencv tutorials at: https://www.intorobotics.com/opencv-tutorials-best-of/


// 1) HISTOGRAM comparison:
//
// http://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_comparison/histogram_comparison.html, http://www.pyimagesearch.com/2014/07/14/3-ways-compare-histograms-using-opencv-python/


// 2) SIFT
// http://aishack.in/tutorials/sift-scale-invariant-feature-transform-introduction/, code at: https://github.com/aishack/

// 3 SUrf with code:
// http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_flann_matcher/feature_flann_matcher.html