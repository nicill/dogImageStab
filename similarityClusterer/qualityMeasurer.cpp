//
// Created by tokuyama on 17/02/03.
//

#include "qualityMeasurer.h"

/**
 * Scores the quality of the clustering when compared to tag files.
 * @param pathToTagFileDirectory A directory containing tag files. Must be a valid directory.
 * @param determinedClusterFrameInfos The clustering to be evalued.
 * @param verbose Activate verbosity to cout.
 * @return A quality score in [0,1].
 */
double qualityMeasurer::scoreQuality(string pathToTagFileDirectory,
                                     vector<vector<FrameInfo>> determinedClusterFrameInfos,
                                     bool verbose) {
    vector<double> percentageMatchedPerFile;
    vector<double> qualityScoresPerFile;
    vector<ClusterInfo> determinedClusters = frameInfosToClusterInfo(determinedClusterFrameInfos);

    DIR *tagFileDir = opendir(pathToTagFileDirectory.c_str());
    struct dirent *fileEntity;
    while ((fileEntity = readdir(tagFileDir)) != nullptr) {
        // Necessary for comparison with string literals.
        string fileName(fileEntity->d_name);
        if (fileName == "." || fileName == "..") {
            if (verbose) cout << "Skipping \"" << fileName << "\"." << endl;
            continue;
        }
        if (verbose) cout << "Reading file \"" << fileName << "\"." << endl;

        vector<ClusterInfo> clustersFromFile = readTagFile(pathToTagFileDirectory + "/" + fileName);

        if (clustersFromFile.size() == 0) {
            cerr << "Couldn't open file " << fileName << " as tag file. Skipping..." << endl;
            continue;
        }

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
        for (ClusterInfo clusterFromFile : clustersFromFile) {
            vector<double> overlapScoresOfDC;
            for (ClusterInfo determinedCluster : determinedClusters) {
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
            // Exemplary scores: No overlap => 0, Perfect overlap of exactly one cluster => 1
            scoresForCFF.push_back(sumOfOverlaps * (1 - malus));
        }

        double averageQualityScoreForCFF = 0;
        for (double qualityScore : scoresForCFF) {
            averageQualityScoreForCFF += qualityScore / scoresForCFF.size();
        }

        cout << "Calculated quality score of " << averageQualityScoreForCFF << " in file \"" << fileName << "\"." << endl;
        qualityScoresPerFile.push_back(averageQualityScoreForCFF);




        // TODO old overlap calculation
        // Try to match the clusters from the tag files to the determined clusters. We aim to find
        // out what percentage of the clusters determined before overlaps with actual clusters.
        double clustersTotalMsec = 0; // total msec of determined clusters
        for (ClusterInfo clusterInfo : determinedClusters) {
            clustersTotalMsec += clusterInfo.length;
        }
        double clustersMatchedMsec = 0; // msec of overlap between determined and actual clusters

        vector<ClusterInfo>::iterator determinedClustersIterator = determinedClusters.begin();
        vector<ClusterInfo>::iterator clustersFromFileIterator = clustersFromFile.begin();
        while (determinedClustersIterator != determinedClusters.end()
               && clustersFromFileIterator != clustersFromFile.end()) {
            if ((*determinedClustersIterator).beginMsec >= (*clustersFromFileIterator).endMsec) {
                clustersFromFileIterator++;
            }

            if ((*determinedClustersIterator).endMsec <= (*clustersFromFileIterator).beginMsec) {
                determinedClustersIterator++;
            }

            if ((*determinedClustersIterator).beginMsec >= (*clustersFromFileIterator).endMsec
                || (*determinedClustersIterator).endMsec <= (*clustersFromFileIterator).beginMsec) {
                continue;
            }

            // Logic: If an overlap exists (neither is the begin after the end nor the end before the begin)
            //        it is between the bigger begin and the smaller end value.
            double biggerBegin =
                    ((*determinedClustersIterator).beginMsec > (*clustersFromFileIterator).beginMsec)
                    ? (*determinedClustersIterator).beginMsec
                    : (*clustersFromFileIterator).beginMsec;
            double smallerEnd =
                    ((*determinedClustersIterator).endMsec < (*clustersFromFileIterator).endMsec)
                    ? (*determinedClustersIterator).endMsec
                    : (*clustersFromFileIterator).endMsec;

            double matchedLength = smallerEnd - biggerBegin;
            clustersMatchedMsec += matchedLength;

            // Iterate the cluster that has been matched to its end.
            // Caution! Assumes that the clusters in both lists do NOT overlap with others in their list!
            if (smallerEnd == (*determinedClustersIterator).endMsec) {
                determinedClustersIterator++;
            }
            else clustersFromFileIterator++;
        }

        double ratioMatched = clustersMatchedMsec / clustersTotalMsec;
        cout << "Matched " << ratioMatched * 100 << " % of clusters in file \"" << fileName << "\"." << endl;
        percentageMatchedPerFile.push_back(ratioMatched);
    }

    closedir(tagFileDir);

    // Calculate average quality score based on all tag files. Option to prioritise files could be necessary.
    double averageQualityScore = 0;
    for (double qualityScoreInFile : qualityScoresPerFile) {
        averageQualityScore += qualityScoreInFile / qualityScoresPerFile.size();
    }

    return averageQualityScore;
}

vector<ClusterInfo> qualityMeasurer::frameInfosToClusterInfo(vector<vector<FrameInfo>> frameInfosList) {
    vector<ClusterInfo> clusterings;

    for (vector<FrameInfo> frameInfos : frameInfosList) {
        double beginMsec = frameInfos.front().msec;
        double endMsec = frameInfos.back().msec;
        clusterings.push_back(
                ClusterInfo(
                        "Cluster from " + to_string((int)beginMsec) + " to " + to_string((int)endMsec),
                        beginMsec,
                        endMsec));
    }

    return clusterings;
}

/**
 * Tries to read the file at the given path as a tag file and interpret its clusterings.
 * @param pathToTagFile The path, including the file name.
 * @return A vector of clusterings if successful, otherwise an empty vector.
 */
vector<ClusterInfo> qualityMeasurer::readTagFile(string pathToTagFile) {
    ifstream tagFile;
    tagFile.open(pathToTagFile);
    if (!tagFile.is_open()) return vector<ClusterInfo>();

    vector<ClusterInfo> clustersFromFile = vector<ClusterInfo>();
    string line;
    while (getline(tagFile, line)) {
        // Check correctness with RegEx?
        vector<string> split = splitLine(line);

        // Split should contain: Name, start (msec), end (msec), duration (msec)
        assert(split.size() == 4);
        char* end;
        double beginMsec = strtod(split[1].c_str(), &end);
        double endMsec = strtod(split[2].c_str(), &end);
        assert(strtod(split[3].c_str(), &end) == (endMsec - beginMsec));

        clustersFromFile.push_back(ClusterInfo(split[0], beginMsec, endMsec));
    }

    tagFile.close();
    return clustersFromFile;
}

vector<string> qualityMeasurer::splitLine(string inputString) {
    vector<string> splitString;
    bool building = false;
    int baseIndex = 0;

    for (int i = 0; i < inputString.length(); i++) {
        char current = inputString[i];
        if (current == ' ' || current == '\t' || current == '\r') {
            if (building) {
                splitString.push_back(inputString.substr((size_t)baseIndex, (size_t)i - baseIndex));
                building = false;
            }

            baseIndex = i + 1;
            continue;
        }

        building = true;
    }

    return splitString;
}
