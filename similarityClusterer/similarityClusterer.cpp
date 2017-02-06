//
// Created by yago on 17/01/20.
//

#include <dirent.h>
#include <opencv2/opencv.hpp>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "opencvImageMetric.h"
#include "featureComparer.h"
#include "qualityMeasurer.h"

using namespace std;
using namespace cv;

// Declarations
double getClusterAverage(vector<FrameInfo>);

int main(int argc, char **argv) {
    bool verbose = false;
    bool computerReadable = false;
    bool qualityMode = false;
    string validUsage = "./computeMeasures [video] [metricIndex] [sub specifier] (optional [flag])";

    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Usage:" << endl
             << validUsage << endl
             << "./computeMeasures --help" << endl
             << endl
             << "Arguments:" << endl
             << "-v: Enable verbose output." << endl
             << "-l: Computer readable output." << endl
             << "-q: Print quality score." << endl;
        return 0;
    }

    // Check for flag
    string last_arg = string(argv[argc - 1]);
    bool hasFlag = last_arg.find("-") != string::npos;

    // Valid cases: Has flag and 5 arguments, or 4 arguments.
    if (!((hasFlag && argc == 5) || argc == 4)) {
        cerr << "Usage: " << validUsage << endl;
        return 1;
    }

    // Try opening the video
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }

    // Read provided flag, if given
    if (hasFlag) {
        if (last_arg == "-v") {
            verbose = true;
            cout << "Verbose output activated." << endl;
        }
        else if (last_arg == "-l") {
            computerReadable = true;
        }
        else if (last_arg == "-q") {
            qualityMode = true;
            cout << "Quality score will be calculated." << endl;
        }
    }

    string pathToTagFiles = "";
    if (qualityMode) {
        string defaultDir = "/home/tokuyama/dog/tags";
        cout << "Please input the directory which contains the tag files (\"d\" for \"" << defaultDir << "\")..." << endl;
        cin >> pathToTagFiles;
        if (pathToTagFiles == "d") pathToTagFiles = defaultDir;

        DIR *tagFileDir = opendir(pathToTagFiles.c_str());
        if (tagFileDir == nullptr || readdir(tagFileDir) == nullptr) {
            cerr << "Could not open directory for tag files supplied: " << pathToTagFiles << endl;
            return 1;
        }

        closedir(tagFileDir);
    }

    // Setting indices
    int metricIndex = atoi(argv[2]);
    int subSpecifier = atoi(argv[3]);

    Mat current, previous;
    framewiseSimilarityMetric *comparer;

    switch (metricIndex) {
        case 1  :
            comparer = new opencvHistComparer(subSpecifier);
            if(verbose)
            {
                // http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
                cout << "Computing histogram measures with value " << subSpecifier << endl;
                cout << "Options - 0: Correlation, 1: Chi-square, 2: Intersection, 3: Bhattacharyya" << endl;
            }
            break;
        case 2 :
            comparer = new featureComparer(featureComparer::SIFT, featureComparer::BF_L2);
            break;
        case 3:
            comparer = new opencvImageMetric(subSpecifier);
            if(verbose)
            {
                cout << "Computing image metrics with value " << subSpecifier << endl;
                cout << "Options - 0: PSNR, 1: SSIM" << endl;
            }
            break;
        default : // Optional
            cerr << "Invalid metric index provided: " << metricIndex << endl;
            return 1;
    }

    if (verbose) comparer->activateVerbosity();

    // Framewise metric computation
    int frameCounter = 1;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    vector<FrameInfo> frameInfos;

    // Load first frame
    capture >> previous;
    frameInfos.push_back(FrameInfo(previous, 1, 0));
    for (;;) {
        capture >> current;

        if (current.data == NULL) {
            break;
        }

        frameCounter++;

        // TODO temp (select section of video to compute)
//        if (frameCounter < 990) continue; // 1371
//        else if (frameCounter > 1440) break; // 1429

        double currentSimilarity = comparer->computeSimilarity(&previous, &current);
        frameInfos.push_back(FrameInfo(current, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity));

        current.copyTo(previous);

        if (verbose) {
            cout << "Similarity " << currentSimilarity
                 << " for frame #" << frameCounter << "/" << totalFrames
                 << " at " << capture.get(CAP_PROP_POS_MSEC) << " msec"
                 << " compared to the last frame" << endl;
        }
    }

    // ----- Clustering -----
    // Get average similarity for region
    int maxIndex = (int) frameInfos.size() - 1;
    for (int i = 0; i < maxIndex; i++) {
        // calculate the average for each frame (5 back, 5 front)
        int start = 0;
        int end = maxIndex;

        if (i >= 5) start = i - 5;
        if (i <= maxIndex - 5) end = i + 5;

        double summedUpSimilarities = 0;
        for (int j = start; j <= end; j++) {
            if (frameInfos[j].similarityToPrevious != -1) {
                summedUpSimilarities += frameInfos[j].similarityToPrevious;
            }
        }
        frameInfos[i].averageSimilarity = summedUpSimilarities / (1 + end - start);
    }

    if (computerReadable) {
        cout << "Cluster,Frame no.,Milliseconds,Similarity to last frame,Average similarity in region" << endl;
    }

    // Find clusters
    vector<vector<FrameInfo>> clusters;
    int currentCluster = 0;
    clusters.push_back(vector<FrameInfo>());
    clusters[currentCluster].push_back(frameInfos[0]);
    double currentClusterAverage = frameInfos[currentCluster].averageSimilarity;

    for (int i = 1; i < frameInfos.size(); i++) {
        // If the difference is too big, we create a new cluster.
        if (abs(currentClusterAverage - frameInfos[i].averageSimilarity) > 0.1) {
            currentCluster++;
            clusters.push_back(vector<FrameInfo>());
        }

        clusters[currentCluster].push_back(frameInfos[i]);
        currentClusterAverage = getClusterAverage(clusters[currentCluster]);

        if (computerReadable) {
            string separator = ",";
            cout << currentCluster << separator
                 << frameInfos[i].frameNum << separator
                 << frameInfos[i].msec << separator
                 << frameInfos[i].similarityToPrevious << separator
                 << frameInfos[i].averageSimilarity << endl;
        }
    }

    if (qualityMode) {
        // TODO what to do with the score? Where to display?
        double score = qualityMeasurer::scoreQuality(pathToTagFiles, clusters, verbose);
        cout << "Achieved a quality score of " << score << " for the video " << argv[1] << endl;
    }

    delete comparer;
    return 0;
}

/**
 * Takes the given FrameInfo objects and calculates the average of their average similarity value.
 * @param clusteredInfos FrameInfo objects.
 * @return Average of all average similarity values.
 */
double getClusterAverage(vector<FrameInfo> clusteredInfos) {
    double summedUpAverages = 0;
    for (FrameInfo frameInfo : clusteredInfos) {
        summedUpAverages += frameInfo.averageSimilarity;
    }
    return summedUpAverages / clusteredInfos.size();
}
