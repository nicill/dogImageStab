//
// Created by tokuyama on 17/02/21.
//

#include "clusterer.h"

using std::cout;
using std::endl;
using std::to_string;

/**
 * Cluster the given frameInfos with the givenStrategy as one clustering.
 * @param givenStrategy A strategy to follow. No internal sanity check!
 * @param frameInfos Frames to cluster.
 */
ClusterInfoContainer clusterer::cluster(strategy givenStrategy, vector<FrameInfo> frameInfos, bool verbose) {
    switch (givenStrategy) {
        case clusterer::AVERAGE:
            return clusterAverage(frameInfos);
        case clusterer::AVERAGE_REFINED:
            return clusterAverageRefined(frameInfos, verbose);
        case clusterer::LABELS:
            return clusterLabels(frameInfos);
        default:
            throw("Clustering strategy not implemented yet!");
    }
}

/**
 * Cluster the given frameInfos with a label-based clustering and group based on label.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
vector<ClusterInfoContainer> clusterer::group(ClusterInfoContainer clustering) {
    vector<ClusterInfoContainer> groupedClusterings;

    for (int i = 1; i < clustering.size(); i++) {
        string name = clustering.clusterInfos[i].label;
        bool found = false;

        for (int j = 0; j < groupedClusterings.size(); j++) {
            // Container with name exists.
            if (name == groupedClusterings[j].name) {
                groupedClusterings[j].add(clustering.clusterInfos[i]);
                found = true;
                break;
            }
        }

        // Name hasn't been seen before.
        if (!found) groupedClusterings.push_back(ClusterInfoContainer(name, clustering.clusterInfos[i]));
    }

    return groupedClusterings;
}

/**
 * Fills the averageSimilarity field of the given FrameInfo objects with a region average
 * (including the 5 before and the 5 after it).
 * @param frameInfos Pointer to the FrameInfo vector to be modified.
 */
void clusterer::calculateRegionAverage(vector<FrameInfo>* frameInfos) {
    // Get average similarity for region and fill field in FrameInfo
    int maxIndex = (int) (*frameInfos).size() - 1;
    for (int i = 0; i < (*frameInfos).size(); i++) {
        // calculate the average for each frame (5 back, 5 front)
        int start = 0;
        int end = maxIndex;

        if (i >= 5) start = i - 5;
        if (i <= maxIndex - 5) end = i + 5;

        double summedUpSimilarities = 0;
        for (int j = start; j <= end; j++) {
            if ((*frameInfos)[j].similarityToPrevious != -1) {
                summedUpSimilarities += (*frameInfos)[j].similarityToPrevious;
            }
        }
        (*frameInfos)[i].averageSimilarity = summedUpSimilarities / (1 + end - start);
    }
}

/**
 * Cluster the given frameInfos with an average similarity clustering.
 * @param frameInfos Frames to cluster.
 * @param verbose Activate verbosity to cout.
 */
vector<ClusterInfo> clusterer::clusterAverageVector(vector<FrameInfo> frameInfos) {
    int currentCluster = 0;
    vector<ClusterInfo> clusters;

    // Always add first frame of first cluster to allow for comparison.
    clusters.push_back(ClusterInfo("0", frameInfos.front()));

    for (int i = 1; i < frameInfos.size(); i++) {
        // Make sure the average similarity has been set
        assert(clusters[currentCluster].averageSimilarity != -1 && frameInfos[i].averageSimilarity != -1);

        // If the difference is too big, we create a new cluster, otherwise we add to the current one.
        if (0.1 < fabs(clusters[currentCluster].averageSimilarity - frameInfos[i].averageSimilarity)) {
            currentCluster++;
            clusters.push_back(ClusterInfo(to_string(currentCluster), frameInfos[i]));
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
ClusterInfoContainer clusterer::clusterAverage(vector<FrameInfo> frameInfos) {
    vector<ClusterInfo> clusters = clusterAverageVector(frameInfos);
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
        // Cluster
        vector<ClusterInfo> newClusters = clusterAverageVector(frameInfos);

        // Update average similarity
        for (int i = 0; i < newClusters.size(); i++) {
            assert(newClusters[i].hasFrames);
            for (int j = 0; j < newClusters[i].frames.size(); j++) {
                assert(newClusters[i].averageSimilarity != -1);
                newClusters[i].frames[j].averageSimilarity = newClusters[i].averageSimilarity;
            }
        }

        // Check if stable clustering was reached
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
ClusterInfoContainer clusterer::clusterLabels(vector<FrameInfo> frameInfos) {
    int currentCluster = 0;
    vector<ClusterInfo> clusters;

    // Always add first frame of first cluster to allow for comparison.
    clusters.push_back(ClusterInfo(frameInfos.front().label, frameInfos.front()));

    for (int i = 1; i < frameInfos.size(); i++) {
        if (clusters[currentCluster].label != frameInfos[i].label) {
            currentCluster++;
            clusters.push_back(ClusterInfo(frameInfos[i].label, frameInfos[i]));
        } else {
            clusters[currentCluster].addFrameAtBack(frameInfos[i]);
        }
    }

    ClusterInfoContainer clustering =
            ClusterInfoContainer("Determined clusters (label-based clustering)", clusters);
    return clustering;
}
