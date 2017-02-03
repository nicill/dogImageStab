//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_FRAMEINFO_CPP
#define DOGIMAGESTABILIZATION_FRAMEINFO_CPP

#include <opencv2/opencv.hpp>

using cv::Mat;

struct FrameInfo
{
    Mat frame = Mat();
    double frameNum = -1;
    double msec = -1;
    double similarityToPrevious = -1;
    double averageSimilarity = -1;

    FrameInfo() {}
    FrameInfo(Mat _frame,
              double _frameNum,
              double _msec,
              double _similarityToPrevious = -1,
              double _averageSimilarity = -1) {
        _frame.copyTo(this->frame);
        this->frameNum = _frameNum;
        this->msec = _msec;
        this->similarityToPrevious = _similarityToPrevious;
        this->averageSimilarity = _averageSimilarity;
    }
};

#endif // DOGIMAGESTABILIZATION_FRAMEINFO_CPP
