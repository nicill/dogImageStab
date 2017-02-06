//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_QUALITYMEASURER_H
#define DOGIMAGESTABILIZATION_QUALITYMEASURER_H

#include <sys/types.h>
#include <dirent.h>
#include <regex>
#include "FrameInfo.cpp"
#include "ClusterInfo.cpp"

using namespace std;

class qualityMeasurer {
public:
    static double scoreQuality(string pathToTagFileDirectory,
                               vector<vector<FrameInfo>> determinedClusterFrameInfos,
                               bool verbose = false);

private:
    static vector<ClusterInfo> frameInfosToClusterInfo(vector<vector<FrameInfo>> frameInfosList);
    static vector<ClusterInfo>* readTagFile(string pathToTagFile);
    static vector<string> splitLine(string inputString);
};

#endif // DOGIMAGESTABILIZATION_QUALITYMEASURER_H
