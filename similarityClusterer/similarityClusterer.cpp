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
    bool timeMode = false;
    bool useDefaults = false;
    string validUsage = "./similarityClusterer <video> <metric index> <sub specifier> [flag(s)]";
    // http://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=comparehist#comparehist
    string subSpecHist = "0: Correlation, 1: Chi-square, 2: Intersection, 3: Bhattacharyya";
    string subSpecFeat = "0: SIFT + BF_L2, 1: SURF + BF_L2, 2: ORB + BF_HAMMING";
    string subSpecImg = "0: PSNR, 1: SSIM";

    if (argc == 2 && string(argv[1]) == "--help") {
        cout << "Usage:" << endl
             << validUsage << endl
             << "./similarityClusterer --help" << endl
             << endl
             << "Metric index         | Sub specifiers" << endl
             << "-------------------------------------------" << endl
             << "1: Histogram         | " << subSpecHist << endl
             << "2: Feature detection | " << subSpecFeat << endl
             << "3: Image metrics     | " << subSpecImg << endl
             << endl
             << "Flags (all lowercase flags can be combined, e.g. '-tq'):" << endl
             << "-v: Enable verbose output." << endl
             << "-q: Print quality score." << endl
             << "-t: Measure time taken." << endl
             << "-d: Use default values for all options." << endl
             << "    --- OR ---" << endl
             << "-L: Computer readable output." << endl;
        return 0;
    }

    // Check for flag
    string last_arg = string(argv[argc - 1]);
    bool hasFlag = last_arg.find("-") == 0;

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
        if (last_arg.find('L') != string::npos) {
            computerReadable = true;
        }
        else {
            if (last_arg.find('v') != string::npos) {
                verbose = true;
                cout << "Verbose output activated." << endl;
            }
            if (last_arg.find('q') != string::npos) {
                qualityMode = true;
                cout << "Quality score will be calculated." << endl;
            }
            if (last_arg.find('t') != string::npos) {
                timeMode = true;
                cout << "Time taken will be measured." << endl;
            }
            if (last_arg.find('d') != string::npos) {
                useDefaults = true;
                cout << "Default values will be used." << endl;
            }
        }
    }

    string pathToTagFiles = "";
    if (qualityMode) {
        string defaultDir = "/home/tokuyama/dog/tags";
        if (!useDefaults) {
            cout << "Please input the directory which contains the tag files (\"d\" for \"" << defaultDir << "\")..." << endl;
            cin >> pathToTagFiles;
        }
        if (useDefaults || pathToTagFiles == "d") pathToTagFiles = defaultDir;

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

    string metricName = "NOT SET";
    string subSpecs = "NOT SET";
    framewiseSimilarityMetric *comparer;
    switch (metricIndex) {
        case 1  :
            comparer = new opencvHistComparer(subSpecifier);
            metricName = "histogram comparison";
            subSpecs = subSpecHist;
            break;
        case 2 :
            if (subSpecifier < 0 && subSpecifier > 2) {
                cerr << "Invalid sub specifier provided: " << subSpecifier << " (Use one of: " << subSpecFeat << ")" << endl;
                return 1;
            }
            comparer = new featureComparer((featureComparer::type)subSpecifier);
            metricName = "feature matching";
            subSpecs = subSpecFeat;
            break;
        case 3:
            comparer = new opencvImageMetric(subSpecifier);
            metricName = "image metric comparison";
            subSpecs = subSpecImg;
            break;
        default : // Optional
            cerr << "Invalid metric index provided: " << metricIndex << endl;
            return 1;
    }

    if (verbose) {
        comparer->activateVerbosity();

        cout << "Using " << metricName << " with sub specifier " << subSpecifier << endl;
        cout << "Possible sub specifiers for this metric:" << endl << subSpecs << endl;
    }

    // Get start time (after user i/o) for timeMode
    time_t startTime = time(nullptr);

    // Framewise metric computation
    Mat current, previous;
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

    current.~Mat();
    previous.~Mat();
    capture.~VideoCapture();

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

    // TODO temp (?)
    time_t similarityFinishedTime = time(nullptr);
    for (double t = 10; t <= 35; t+=5) {

        // Find clusters
        vector<vector<FrameInfo>> clusters;
        int currentCluster = 0;
        clusters.push_back(vector<FrameInfo>());
        clusters[currentCluster].push_back(frameInfos[0]);
        double currentClusterAverage = frameInfos[currentCluster].averageSimilarity;

        for (int i = 1; i < frameInfos.size(); i++) {
            // If the difference is too big, we create a new cluster.
            if (abs(currentClusterAverage - frameInfos[i].averageSimilarity) > (t/100)) {
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

        // TODO temp
        cout << "------------------" << endl << "USING CUT OFF " << (t/100) << endl;

        if (qualityMode) {
            double score = qualityMeasurer::scoreQuality(pathToTagFiles, clusters, verbose);
            cout << "Achieved a quality score of " << score << " for the video \"" << argv[1] << "\"!" << endl;
        }

    }
    // TODO END TODO

    time_t qualityFinishedTime = time(nullptr);

    if (timeMode) {
        cout    << "----------" << endl
                << "Time taken" << endl
                << "Total: " << qualityFinishedTime - startTime << " s" << endl
                << "Similarity measurement: " << similarityFinishedTime - startTime << " s" << endl
                << "Quality measurement: " << qualityFinishedTime - similarityFinishedTime << " s" << endl;
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
