//
// Created by yago on 17/05/16.
//

#ifndef DOGIMAGESTABILIZATION_TESTVIDEOGENERATOR_H
#define DOGIMAGESTABILIZATION_TESTVIDEOGENERATOR_H

#include <iostream>
#include <stdio.h>
#include <string>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

class testVideoGenerator {

public:
    void generateImageRotation(string imagePath,double rotAngle, string outputVideoPath);


};


#endif //DOGIMAGESTABILIZATION_TESTVIDEOGENERATOR_H
