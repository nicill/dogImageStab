//
// Created by yago on 17/01/23.
//


#ifndef DOGIMAGESTABILIZATION_OPENCVHISTCOMPARER_H
#define DOGIMAGESTABILIZATION_OPENCVHISTCOMPARER_H


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "framewiseSimilarityMetric.h"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;


class opencvHistComparer: public framewiseSimilarityMetric {

public:
    double computeSimilarity(Mat* im1, Mat* im2);
    opencvHistComparer(int i=2);

private:
    int comparisonType;

};


#endif //DOGIMAGESTABILIZATION_OPENCVHISTCOMPARER_H
