//
// Created by tokuyama on 17/02/21.
//

#ifndef DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
#define DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H

#include <vector>
#include "storageClasses/FrameInfo.cpp"
#include "storageClasses/ClusterInfoContainer.cpp"

using std::vector;

class clusterer {
public:
    enum strategy { AVERAGE, AVERAGE_REFINED, LABELS };

    static ClusterInfoContainer cluster(strategy givenStrategy, vector<FrameInfo> frameInfos, bool verbose);
    static vector<ClusterInfoContainer> group(ClusterInfoContainer clustering);

private:
    static vector<ClusterInfo> clusterAverageVector(vector<FrameInfo> frameInfos);
    static ClusterInfoContainer clusterAverage(vector<FrameInfo> frameInfos);
    static ClusterInfoContainer clusterAverageRefined(vector<FrameInfo> frameInfos, bool verbose);
    static ClusterInfoContainer clusterLabels(vector<FrameInfo> frameInfos);
};

#endif //DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
