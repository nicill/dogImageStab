//
// Created by tokuyama on 17/02/03.
//

#include "qualityMeasurer.h"

/**
 * Scores the quality of the clustering when compared to tag files.
 * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
 * @param determinedClusterFrameInfos The clustering to be evaluated.
 * @param verbose Activate verbosity to cout.
 * @return A quality score in [0,1].
 */
double qualityMeasurer::scoreQuality(string pathToTagFileDirectory,
                                     ClusterInfoContainer determinedClusters,
                                     bool verbose) {
    vector<double> qualityScoresPerFile;

    vector<ClusterInfoContainer> clustersFromAllFiles = similarityFileUtils::readTagFiles(pathToTagFileDirectory, verbose);
    for (ClusterInfoContainer clustersFromFile : clustersFromAllFiles) {
        double qualityScoreForFile = getQualityScore(clustersFromFile, determinedClusters);
        double precision = getClusterOverlapPrecision(clustersFromFile, determinedClusters);
        double recall = getClusterOverlapRecall(clustersFromFile, determinedClusters);

        cout << "Quality score " << qualityScoreForFile
             << " (" << precision * 100 << " % precision, " << recall * 100 << " % recall) "
             << "for ground truth \"" << clustersFromFile.name << "\"." << endl;
        qualityScoresPerFile.push_back(qualityScoreForFile);
    }

    // Calculate average quality score based on all tag files. Option to prioritise files could be necessary.
    double averageQualityScore = 0;
    for (double qualityScoreInFile : qualityScoresPerFile) {
        averageQualityScore += qualityScoreInFile / qualityScoresPerFile.size();
    }

    return averageQualityScore;
}

/**
 * Calculates the overlap of the given frames with each of the clusterings in the given tag files and outputs it.
 * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
 * @param frames The frames to be evaluated.
 * @param verbose Activate verbosity to cout.
 */
void qualityMeasurer::calculateOverlap(string pathToTagFileDirectory, vector<FrameInfo> frames, bool verbose) {
    vector<ClusterInfoContainer> clustersFromAllFiles = similarityFileUtils::readTagFiles(pathToTagFileDirectory, verbose);
    for (ClusterInfoContainer clustersFromFile : clustersFromAllFiles) {
        double overlappingFrames = 0;
        for (ClusterInfo cluster : clustersFromFile.clusterInfos) {
            for (FrameInfo frame : frames) {
                if (frame.msec >= cluster.beginMsec && frame.msec <= cluster.endMsec) {
                    overlappingFrames++;
                }
            }
        }
        double overlapRatio = overlappingFrames / frames.size();
        cout << overlapRatio * 100 << " % of frames overlap clusters in \"" << clustersFromFile.name << "\"" << endl;
    }
}

/**
 * Calculates the overlap of labels in the given tag files and outputs it.
 * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
 * @param verbose Activate verbosity to cout.
 */
void qualityMeasurer::calculateOverlap(string pathToTagFileDirectory, bool verbose) {
    vector<ClusterInfoContainer> clustersFromAllFiles = similarityFileUtils::readTagFiles(pathToTagFileDirectory, verbose);

    for (int i = 0; i < clustersFromAllFiles.size(); i++) {
        for (int j = i + 1; j < clustersFromAllFiles.size(); j++) {
            double overlapMsec = getClusterOverlapMsec(clustersFromAllFiles[i], clustersFromAllFiles[j]);

            if (overlapMsec == 0) {
                cout << "No overlap of " << clustersFromAllFiles[i].name << " and " << clustersFromAllFiles[j].name
                     << endl;
                continue;
            }

            cout << "Overlap of " << clustersFromAllFiles[i].name << " and " << clustersFromAllFiles[j].name << ": "
                 << overlapMsec << " msec" << endl;

            if (overlapMsec == 0) continue;

            double percentageI = getClusterOverlapPrecision(clustersFromAllFiles[i], clustersFromAllFiles[j]);
            double percentageJ = getClusterOverlapPrecision(clustersFromAllFiles[j], clustersFromAllFiles[i]);
            cout << " -> " << percentageI * 100 << " % of " << clustersFromAllFiles[j].name << " matches "
                 << clustersFromAllFiles[i].name << endl
                 << " -> " << percentageJ * 100 << " % of " << clustersFromAllFiles[i].name << " matches "
                 << clustersFromAllFiles[j].name << endl;
        }
    }
}

/**
 * Calculates a quality score with a ground truth and an estimation.
 * @param groundTruthClusters Ground truth to compare to.
 * @param evaluatedClusters Estimation that will be evaluated.
 * @return Quality score in ]-inf,1]
 */
double qualityMeasurer::getQualityScore(ClusterInfoContainer groundTruthClusters,
                                        ClusterInfoContainer evaluatedClusters) {
    // For each cluster in tag file:
    //    For each determined cluster:
    //       if (overlap):                  (given clustering:         |-------|
    //          1.) Calculate overlap in %  (determined clust.:     |~~~~~|       => 30 % overlap
    //          2.) Calculate ratio of overlap vs. non overlap:     non|yes       => 50/50, aka 50 %
    //          3.) Get overlap score (overlap % * overlap ratio, e.g. 30 % * 50 % = 15 %)
    //    malus = 0
    //    For each overlapping cluster additional to the best match:
    //       if (overlap / best overlap) > 0.1:
    //          malus += 25 * (overlap / best overlap) (in ]2.5,25])
    //    Add up all overlaps and multiply by (100 - malus) - can be negative!
    vector<double> scoresForCFF;
    for (ClusterInfo clusterFromFile : groundTruthClusters.clusterInfos) {
        vector<double> overlapScoresOfDC;
        for (ClusterInfo determinedCluster : evaluatedClusters.clusterInfos) {
            if (determinedCluster.endMsec <= clusterFromFile.beginMsec
                || determinedCluster.beginMsec >= clusterFromFile.endMsec) {
                continue;
            }

            // Logic: If an overlap exists (neither is the begin after the end nor the end before
            //        the begin) it is between the bigger begin and the smaller end value.
            double biggerBegin =
                    (determinedCluster.beginMsec > clusterFromFile.beginMsec)
                    ? determinedCluster.beginMsec
                    : clusterFromFile.beginMsec;
            double smallerEnd =
                    (determinedCluster.endMsec < clusterFromFile.endMsec)
                    ? determinedCluster.endMsec
                    : clusterFromFile.endMsec;
            double overlapLength = smallerEnd - biggerBegin;

            // Percentage of cluster from file that is overlapped
            assert(clusterFromFile.length != 0);
            double overlapPercent = overlapLength / clusterFromFile.length;
            // Ratio of overlap vs. non overlap in determined cluster
            double overlapRatioInDC = 0;
            if (determinedCluster.length != 0) overlapRatioInDC = overlapLength / determinedCluster.length;
            // Overlap score (overlap percentage * overlap ratio)
            overlapScoresOfDC.push_back(overlapPercent * overlapRatioInDC);
        }

        // Get best overlap
        double bestOverlapScore = 0;
        double sumOfOverlaps = 0;
        for (double overlapScore : overlapScoresOfDC) {
            sumOfOverlaps += overlapScore;

            if (overlapScore > bestOverlapScore) {
                bestOverlapScore = overlapScore;
            }
        }

        // Sum up overlaps and calculate malus for extra overlaps
        double malus = 0;
        for (double overlapScore : overlapScoresOfDC) {
            // Skip best overlap
            if (overlapScore == bestOverlapScore) {
                continue;
            }

            double ratioOfOverlaps = overlapScore / bestOverlapScore;
            if (ratioOfOverlaps > 0.1) {
                malus += 0.25 * ratioOfOverlaps; // in ]2.5,25]
            }
        }

        // Calculate quality score for cluster (can be negative!)
        // Exemplary scores: No overlap => 0, Perfect overlap of exactly one cluster => 1,
        //                   infinite clusters of equal size => -inf
        scoresForCFF.push_back(sumOfOverlaps * (1 - malus));
    }

    // Could be prioritised based on cluster length.
    double averageQualityScore = 0;
    for (double qualityScore : scoresForCFF) {
        averageQualityScore += qualityScore / scoresForCFF.size();
    }

    return averageQualityScore;
}

/**
 * Calculates the milliseconds of overlap of evaluated clusters and ground truth.
 * @param groundTruthClusters Ground truth to compare to.
 * @param evaluatedClusters Clustering that will be evaluated.
 * @return Total msec of cluster overlaps.
 */
double qualityMeasurer::getClusterOverlapMsec(ClusterInfoContainer groundTruthClusters,
                                              ClusterInfoContainer evaluatedClusters) {
    // Try to match the evaluatedClusters to the groundTruthClusters.
    double clustersMatchedMsec = 0; // msec of overlap between determined and actual clusters

    vector<ClusterInfo>::iterator evaluatedClustersIterator = evaluatedClusters.clusterInfos.begin();
    vector<ClusterInfo>::iterator groundTruthClustersIterator = groundTruthClusters.clusterInfos.begin();
    while (evaluatedClustersIterator != evaluatedClusters.clusterInfos.end()
           && groundTruthClustersIterator != groundTruthClusters.clusterInfos.end()) {
        if ((*evaluatedClustersIterator).beginMsec >= (*groundTruthClustersIterator).endMsec) {
            groundTruthClustersIterator++;
            continue;
        }

        if ((*evaluatedClustersIterator).endMsec <= (*groundTruthClustersIterator).beginMsec) {
            evaluatedClustersIterator++;
            continue;
        }

        ClusterInfo overlap = (*groundTruthClustersIterator).getOverlap((*evaluatedClustersIterator));
        double matchedLength = overlap.endMsec - overlap.beginMsec;
        assert(matchedLength > 0);
        clustersMatchedMsec += matchedLength;

        // Iterate the cluster that has been matched to its end.
        // Caution! Assumes that the clusters in both lists do NOT overlap with others in their list!
        if (overlap.endMsec == (*evaluatedClustersIterator).endMsec) {
            evaluatedClustersIterator++;
        } else {
            groundTruthClustersIterator++;
        }
    }

    return clustersMatchedMsec;
}

/**
 * Calculates the precision of the clustering, which is the percentage of evaluated clusters
 * that overlap with the ground truth.
 * @param groundTruthClusters Ground truth to compare to.
 * @param evaluatedClusters Clustering that will be evaluated.
 * @return Precision ratio in [0,1].
 */
double qualityMeasurer::getClusterOverlapPrecision(ClusterInfoContainer groundTruthClusters,
                                                   ClusterInfoContainer evaluatedClusters) {
    double overlapMsec = getClusterOverlapMsec(groundTruthClusters, evaluatedClusters);
    double evaluatedClustersTotalMsec = getClustersTotalMsec(evaluatedClusters);

    // How many selected items (overlapping clusters) are relevant?
    double precision = overlapMsec / evaluatedClustersTotalMsec;

    return precision;
}

/**
 * Calculates the recall of the clustering, which is the percentage of overlapping clusters
 * relative to the total clusters of the ground truth.
 * @param groundTruthClusters Ground truth to compare to.
 * @param evaluatedClusters Clustering that will be evaluated.
 * @return Recall ratio in [0,1].
 */
double qualityMeasurer::getClusterOverlapRecall(ClusterInfoContainer groundTruthClusters,
                                                ClusterInfoContainer evaluatedClusters) {
    double overlapMsec = getClusterOverlapMsec(groundTruthClusters, evaluatedClusters);
    double groundTruthClustersTotalMsec = getClustersTotalMsec(groundTruthClusters);

    // How many relevant items (overlapping clusters) do overlap?
    double recall = overlapMsec / groundTruthClustersTotalMsec;

    return recall;
}

double qualityMeasurer::getClustersTotalMsec(ClusterInfoContainer clusters) {
    double totalMsec = 0; // total msec of the evaluatedClusters
    for (ClusterInfo clusterInfo : clusters.clusterInfos) {
        totalMsec += clusterInfo.length;
    }
    return totalMsec;
}
