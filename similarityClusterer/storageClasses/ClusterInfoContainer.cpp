//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP

#include <vector>
#include "ClusterInfo.cpp"

using std::string;
using std::vector;

struct ClusterInfoContainer {
    string name = "INVALID";
    vector<ClusterInfo> clusterInfos;

    /**
     * Empty constructor.
     */
    ClusterInfoContainer() {};

    /**
     * Nested constructor for a single element.
     */
    ClusterInfoContainer(string _name, ClusterInfo _clusterInfo)
            : ClusterInfoContainer(_name, utils::package(_clusterInfo)) {}

    /**
     * Constructor.
     */
    ClusterInfoContainer(string _name, vector<ClusterInfo> _clusterInfos = vector<ClusterInfo>()) {
        this->name = _name;
        this->clusterInfos = _clusterInfos;
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP
