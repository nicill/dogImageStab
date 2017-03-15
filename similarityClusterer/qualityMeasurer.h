//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_QUALITYMEASURER_H
#define DOGIMAGESTABILIZATION_QUALITYMEASURER_H

#include <sys/types.h>
#include <dirent.h>
#include <regex>
#include "tagFileUtils.cpp"
#include "storageClasses/FrameInfo.cpp"
#include "storageClasses/ClusterInfo.cpp"
#include "storageClasses/ClusterInfoContainer.cpp"

using namespace std;

class qualityMeasurer {
public:
    static double scoreQuality(string pathToTagFileDirectory,
                               ClusterInfoContainer determinedClusterFrameInfos,
                               bool verbose = false);
    static void calculateOverlap(string pathToTagFileDirectory,
                                 vector<FrameInfo> frames,
                                 bool verbose = false);
    static void calculateOverlap(string pathToTagFileDirectory,
                                 bool verbose = false);

private:
    static double getQualityScore(ClusterInfoContainer groundTruthClusters,
                                  ClusterInfoContainer evaluatedClusters);
    static double getClusterOverlapRecall(ClusterInfoContainer groundTruthClusters,
                                          ClusterInfoContainer evaluatedClusters);
    static double getClusterOverlapMsec(ClusterInfoContainer groundTruthClusters,
                                        ClusterInfoContainer evaluatedClusters);
    static double getClusterOverlapPrecision(ClusterInfoContainer groundTruthClusters,
                                             ClusterInfoContainer evaluatedClusters);
    static double getClustersTotalMsec(ClusterInfoContainer clusters);
};

#endif // DOGIMAGESTABILIZATION_QUALITYMEASURER_H
