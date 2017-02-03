//
// Created by tokuyama on 17/02/03.
//

#include <opencv2/opencv.hpp>

using cv::Mat;

struct FrameInfo
{
    Mat frame;
    double frameNum;
    double msec;
    double similarityToPrevious;
    double averageSimilarity;

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
