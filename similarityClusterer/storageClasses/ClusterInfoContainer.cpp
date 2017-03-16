//
// Created by tokuyama on 17/02/03.
//

#ifndef DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP
#define DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP

#include <vector>
#include "ClusterInfo.cpp"

using std::string;
using std::vector;
using std::stable_sort;

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
        stable_sort(_clusterInfos.begin(), clusterInfos.end(), ClusterInfo::less);
        this->clusterInfos = _clusterInfos;
    }

    /**
     * Add a new cluster, sorted by begin time.
     */
    void add(ClusterInfo newCluster) {
        for (int i = 0; i < this->clusterInfos.size(); i++) {
            if (this->clusterInfos[i].beginMsec > newCluster.beginMsec) {
                this->clusterInfos.insert(this->clusterInfos.begin()+(i-1), newCluster);
                return;
            }
        }

        this->clusterInfos.push_back(newCluster);
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP
