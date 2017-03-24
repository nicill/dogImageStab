//
// Created by tokuyama on 17/02/22.
//

#include "classifier.h"

/**
 * Labels each frame based on its similarityToPrevious field.
 */
void classifier::classifyFramesSingle(vector<FrameInfo>* frames) {
    for (int i = 0; i < (*frames).size(); i++) {
        (*frames)[i].label = classifySimilarity((*frames)[i].similarityToPrevious);
    }
}

/**
 * Labels each frame based on its averageSimilarity field.
 */
void classifier::classifyFramesAverage(vector<FrameInfo>* frames) {
    for (int i = 0; i < (*frames).size(); i++) {
        assert((*frames)[i].averageSimilarity != -1);
        (*frames)[i].label = classifySimilarity((*frames)[i].averageSimilarity);
    }
}

/**
 * Labels each ClusterInfo with a classification.
 */
void classifier::classifyClusters(ClusterInfoContainer* clusters) {
    for (int i = 0; i < (*clusters).size(); i++) {
        (*clusters).clusters[i].label = classifySimilarity((*clusters).clusters[i].averageSimilarity);
    }
}

string classifier::classifySimilarity(double value) {
    if (value > 0.6) return highSimLabel;
    else if (value > 0.3) return mediumSimLabel;
    else return lowSimLabel;
}
