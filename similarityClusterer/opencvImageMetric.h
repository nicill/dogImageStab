//
// Created by yago on 17/02/02.
//

#ifndef DOGIMAGESTABILIZATION_OPENCVIMAGEMETRIC_H
#define DOGIMAGESTABILIZATION_OPENCVIMAGEMETRIC_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "framewiseSimilarityMetric.h"
#include <iostream>
#include <stdio.h>

class opencvImageMetric: public framewiseSimilarityMetric {

public:

double computeSimilarity(Mat* im1, Mat* im2);
opencvImageMetric(int i=0);

private:
int comparisonType;


};


#endif //DOGIMAGESTABILIZATION_OPENCVIMAGEMETRIC_H
