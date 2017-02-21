//
// Created by tokuyama on 17/02/21.
//

#ifndef DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
#define DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H

#include <vector>
#include "FrameInfo.cpp"
#include "ClusterInfoContainer.cpp"

using std::vector;

class frameInfoClusterer {
public:
    enum strategy { AVERAGE, AVERAGE_REFINED, LABELS };

    static ClusterInfoContainer cluster(strategy givenStrategy, vector<FrameInfo> frameInfos);
};

#endif //DOGIMAGESTABILIZATION_FRAMEINFOCLUSTERER_H
