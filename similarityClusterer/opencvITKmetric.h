//
// Created by yago on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_OPENCVITKMETRIC_H
#define DOGIMAGESTABILIZATION_OPENCVITKMETRIC_H


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "framewiseSimilarityMetric.h"
#include <iostream>
#include <stdio.h>

#include <itkImage.h>
#include <itkCastImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkOpenCVImageBridge.h>
#include "itkMeanSquaresImageToImageMetric.h"
#include <itkMinimumMaximumImageCalculator.h>
#include "itkMattesMutualInformationImageToImageMetric.h"
#include <itkNormalizedCorrelationImageToImageMetric.h>

using namespace std;
using namespace cv;


class opencvITKmetric : public framewiseSimilarityMetric {

public:
    opencvITKmetric(int comparisonType=0);
    double computeSimilarity(Mat* im1, Mat* im2);
    void activateVerbosity();


private:
    int comparisonType;

};


#endif //DOGIMAGESTABILIZATION_OPENCVITKMETRIC_H






