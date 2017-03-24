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
    vector<ClusterInfo> clusters;

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
        this->clusters = _clusterInfos;
        this->sortClusters();
    }

    /**
     * Add a new cluster, sorted by begin time.
     */
    void add(ClusterInfo newCluster) {
        this->clusters.push_back(newCluster);
        this->sortClusters();
    }

    /**
     * Returns number of contained clusters.
     */
    size_t size() {
        return this->clusters.size();
    }

    /**
     * Direct accessor to contained ClusterInfo objects.
     */
    ClusterInfo operator [] (int i) {
        return this->clusters[i];
    }

private:
    /**
     * Sorts the contained vector of ClusterInfo objects.
     */
    void sortClusters() {
        stable_sort(this->clusters.begin(), this->clusters.end());
    }
};

#endif // DOGIMAGESTABILIZATION_CLUSTERINFOCONTAINER_CPP
