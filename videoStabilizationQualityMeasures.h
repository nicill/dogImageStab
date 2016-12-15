//
// Created by yago on 16/12/08.
//

#include <opencv2/opencv.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
//#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O

#include <string>
#include <iostream>
#include <iomanip>  // for controlling float print precision


using namespace std;
using namespace cv;


#ifndef DOGIMAGESTABILIZATION_VIDEOSTABILIZATIONQUALITYMEASURES_H
#define DOGIMAGESTABILIZATION_VIDEOSTABILIZATIONQUALITYMEASURES_H


class videoStabilizationQualityMeasures {


public:
    double framewiseEntropy(string videoPath);
    double ITF(string video);
    double blackPixelPercent(string videoPath);
    double SSIM(string videoPath);
};


#endif //DOGIMAGESTABILIZATION_VIDEOSTABILIZATIONQUALITYMEASURES_H
