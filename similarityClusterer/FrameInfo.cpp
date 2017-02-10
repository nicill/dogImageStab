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
    double frameNo = -1;
    double msec = -1;
    double similarityToPrevious = -1;

    std::string label = "NO LABEL";
    double averageSimilarity = -1;

    FrameInfo() {}
    FrameInfo(Mat _frame,
              double _frameNo,
              double _msec,
              double _similarityToPrevious = -1,
              std::string _label = "NO LABEL",
              double _averageSimilarity = -1) {
        _frame.copyTo(this->frame);
        this->frameNo = _frameNo;
        this->msec = _msec;
        this->similarityToPrevious = _similarityToPrevious;

        this->label = _label;
        this->averageSimilarity = _averageSimilarity;
    }
};

#endif // DOGIMAGESTABILIZATION_FRAMEINFO_CPP
