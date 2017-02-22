//
// Created by tokuyama on 17/02/21.
//

#include "clusterer.h"

using std::cout;
using std::endl;
using std::to_string;

/**
 * Cluster the given frameInfos with the givenStrategy.
 * @param givenStrategy A strategy to follow. No internal sanity check!
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
ClusterInfoContainer clusterer::cluster(strategy givenStrategy, vector<FrameInfo> frameInfos, bool verbose) {
    switch (givenStrategy) {
        case clusterer::AVERAGE:
            return clusterAverage(frameInfos, false);
        case clusterer::AVERAGE_REFINED:
            return clusterAverageRefined(frameInfos, false);
        case clusterer::LABELS:
            return clusterLabels(frameInfos, false);
        default:
            throw("Clustering strategy not implemented yet!");
    }
}

/**
 * Cluster the given frameInfos with an average similarity clustering.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
vector<ClusterInfo> clusterer::clusterAverageVector(vector<FrameInfo> frameInfos, bool verbose) {
    int currentCluster = 0;
    vector<ClusterInfo> clusters;

    // Always add first frame of first cluster to allow for comparison.
    clusters.push_back(ClusterInfo("0", { frameInfos.front() }));

    for (int i = 1; i < frameInfos.size(); i++) {
        // If the difference is too big, we create a new cluster, otherwise we add to the current one.
        if (0.1 < fabs(clusters[currentCluster].averageSimilarity - frameInfos[i].averageSimilarity)) {
            currentCluster++;
            clusters.push_back(ClusterInfo(to_string(currentCluster), { frameInfos[i] }));
        } else {
            clusters[currentCluster].addFrameAtBack(frameInfos[i]);
        }
    }

    return clusters;
}

/**
 * Cluster the given frameInfos with an average similarity clustering.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
ClusterInfoContainer clusterer::clusterAverage(vector<FrameInfo> frameInfos, bool verbose) {
    vector<ClusterInfo> clusters = clusterAverageVector(frameInfos, verbose);
    ClusterInfoContainer clustering = ClusterInfoContainer("Determined clusters (average clustering)", clusters);
    return clustering;
}

/**
 * Cluster the given frameInfos with an average similarity clustering and refine until a stable clustering is reached.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
ClusterInfoContainer clusterer::clusterAverageRefined(vector<FrameInfo> frameInfos, bool verbose) {
    vector<ClusterInfo> clusters = { ClusterInfo("All frames", frameInfos) };
    int iteration = 0;
    for (; iteration < 500; iteration++) {
        vector<ClusterInfo> newClusters = clusterAverageVector(frameInfos, verbose);

        for (int i = 0; i < newClusters.size(); i++) {
            assert(newClusters[i].hasFrames);
            for (int j = 0; j < newClusters[i].frames.size(); j++) {
                newClusters[i].frames[j].averageSimilarity = newClusters[i].averageSimilarity;
            }
        }

        bool equal = true;
        vector<ClusterInfo>::iterator clustersIterator = clusters.begin();
        vector<ClusterInfo>::iterator newClustersIterator = newClusters.begin();
        while (clustersIterator != clusters.end()
               && newClustersIterator != newClusters.end()) {
            if (!((*clustersIterator).equals(*newClustersIterator))) {
                equal = false;
                break;
            } else {
                clustersIterator++;
                newClustersIterator++;
            }
        }
        clusters = newClusters;

        if (equal) break;
        if (verbose) cout << "No stable clustering reached, recalculating...";
    }

    if (verbose) cout << "Reached stable clustering in " << iteration << " iterations";

    ClusterInfoContainer clustering =
            ClusterInfoContainer("Determined clusters (average refined clustering)", clusters);
    return clustering;
}

/**
 * Cluster the given frameInfos with a label-based clustering.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
ClusterInfoContainer clusterer::clusterLabels(vector<FrameInfo> frameInfos, bool verbose) {
    int currentCluster = 0;
    vector<ClusterInfo> clusters;

    // Always add first frame of first cluster to allow for comparison.
    clusters.push_back(ClusterInfo(frameInfos.front().label, { frameInfos.front() }));

    for (int i = 1; i < frameInfos.size(); i++) {
        if (clusters[currentCluster].name != frameInfos[i].label) {
            currentCluster++;
            clusters.push_back(ClusterInfo(frameInfos[i].label, { frameInfos[i] }));
        } else {
            clusters[currentCluster].addFrameAtBack(frameInfos[i]);
        }
    }

    ClusterInfoContainer clustering =
            ClusterInfoContainer("Determined clusters (label-based clustering)", clusters);
    return clustering;
}
