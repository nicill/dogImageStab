//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_CLASSIFIER_H
#define DOGIMAGESTABILIZATION_CLASSIFIER_H

#include <vector>
#include "FrameInfo.cpp"

using std::vector;

class classifier {
public:
    // Constants
    static constexpr const char* highSimLabel = "High similarity";
    static constexpr const char* mediumSimLabel = "Medium similarity";
    static constexpr const char* lowSimLabel = "Low similarity";

    static vector<FrameInfo> classifyFrames(vector<FrameInfo> frames);

private:
    enum classification { HIGH, MEDIUM, LOW };

    static classification classifySimilarity(double value);
};

#endif //DOGIMAGESTABILIZATION_CLASSIFIER_H
