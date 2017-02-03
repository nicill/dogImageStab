//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_QUALITYMEASURER_H
#define DOGIMAGESTABILIZATION_QUALITYMEASURER_H

#include "FrameInfo.cpp"

using std::string;
using std::vector;

class qualityMeasurer {
public:
    static void scoreQuality(string pathToTagFiles, vector<vector<FrameInfo>> clusters);
};

#endif //DOGIMAGESTABILIZATION_QUALITYMEASURER_H
