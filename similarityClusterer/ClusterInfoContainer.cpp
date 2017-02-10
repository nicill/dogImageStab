//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFOS_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFOS_CPP

#include <vector>
#include "ClusterInfo.cpp"

struct ClusterInfoContainer {
    std::string name = "INVALID";
    std::vector<ClusterInfo> clusterInfos;

    ClusterInfoContainer() {};
    ClusterInfoContainer(std::string _name, std::vector<ClusterInfo> _clusterInfos = std::vector<ClusterInfo>()) {
        this->name = _name;
        this->clusterInfos = _clusterInfos;
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFOS_CPP
