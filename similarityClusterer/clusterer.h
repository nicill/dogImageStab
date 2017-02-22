//
// Created by tokuyama on 17/02/21.
//

#ifndef DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
#define DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H

#include <vector>
#include "FrameInfo.cpp"
#include "ClusterInfoContainer.cpp"

using std::vector;

class clusterer {
public:
    enum strategy { AVERAGE, AVERAGE_REFINED, LABELS };

    static ClusterInfoContainer cluster(strategy givenStrategy, vector<FrameInfo> frameInfos, bool verbose);
    static vector<ClusterInfoContainer> clusterAndGroup(vector<FrameInfo> frameInfos, bool verbose);

private:
    static vector<ClusterInfo> clusterAverageVector(vector<FrameInfo> frameInfos, bool verbose);
    static ClusterInfoContainer clusterAverage(vector<FrameInfo> frameInfos, bool verbose);
    static ClusterInfoContainer clusterAverageRefined(vector<FrameInfo> frameInfos, bool verbose);
    static ClusterInfoContainer clusterLabels(vector<FrameInfo> frameInfos, bool verbose);
};

#endif //DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
