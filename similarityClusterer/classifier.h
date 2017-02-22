//
// Created by tokuyama on 17/02/22.
//

#ifndef DOGIMAGESTABILIZATION_CLASSIFIER_H
#define DOGIMAGESTABILIZATION_CLASSIFIER_H

#include <vector>
#include "storageClasses/FrameInfo.cpp"
#include "storageClasses/ClusterInfoContainer.cpp"

using std::vector;

class classifier {
public:
    // Constants
    static constexpr const char* highSimLabel = "High similarity";
    static constexpr const char* mediumSimLabel = "Medium similarity";
    static constexpr const char* lowSimLabel = "Low similarity";

    static vector<FrameInfo> classifyFramesSingle(vector<FrameInfo> frames);
    static vector<FrameInfo> classifyFramesAverage(vector<FrameInfo> frames);
    static ClusterInfoContainer classifyClusters(ClusterInfoContainer clusters);

private:
    static string classifySimilarity(double value);
};

#endif //DOGIMAGESTABILIZATION_CLASSIFIER_H
