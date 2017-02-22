//
// Created by tokuyama on 17/02/22.
//

#include "classifier.h"

vector<FrameInfo> classifier::classifyFramesSingle(vector<FrameInfo> frames) {
    for (int i = 0; i < frames.size(); i++) {
        frames[i].label = classifySimilarity(frames[i].similarityToPrevious);
    }

    return frames;
}

vector<FrameInfo> classifier::classifyFramesAverage(vector<FrameInfo> frames) {
    for (int i = 0; i < frames.size(); i++) {
        assert(frames[i].averageSimilarity != -1);
        frames[i].label = classifySimilarity(frames[i].averageSimilarity);
    }

    return frames;
}

ClusterInfoContainer classifier::classifyClusters(ClusterInfoContainer clusters) {
    for (int i = 0; i < clusters.clusterInfos.size(); i++) {
        clusters.clusterInfos[i].label = classifySimilarity(clusters.clusterInfos[i].averageSimilarity);
    }

    return clusters;
}

string classifier::classifySimilarity(double value) {
    if (value > 0.6) return highSimLabel;
    else if (value > 0.3) return mediumSimLabel;
    else return lowSimLabel;
}
