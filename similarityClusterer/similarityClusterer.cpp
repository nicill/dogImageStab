//
// Created by yago on 17/01/20.
//

#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <libgen.h>
#include "framewiseSimilarityMetric.h"
#include "opencvHistComparer.h"
#include "opencvImageMetric.h"
#include "featureComparer.h"
#include "qualityMeasurer.h"
#include "defaults.h"
#include "clusterer.h"
#include "classifier.h"

using namespace std;
using namespace cv;

// Declarations
void clusterRegion(vector<FrameInfo> frames, string pathToTagFiles, bool verbose);
void clusterLabels(vector<FrameInfo> frames, string pathToTagFiles, bool verbose);
void groupAndEvaluate(ClusterInfoContainer clusters, string pathToTagFiles, bool verbose);
void classify(vector<FrameInfo> classifiedFrames, string pathToTagFiles, bool verbose);
void announceMode(string mode);

/**
 * Main method.
 */
int main(int argc, char **argv) {
    bool verbose = false;
    bool measureTime = false;
    bool fileIO = false;
    bool useDefaults = false;

    bool qualityMeasurement = false;
    bool regionClusterMode = false;
    bool classificationClusterMode = false;
    bool frameClassifyMode = false;
    string validUsage = "./similarityClusterer <video> <metric index> <sub specifier> [flag(s)]";
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
             << "Modifier flags (flags can be combined, e.g. '-Ctv'):" << endl
             << "-v: Enable verbose output." << endl
             << "-t: Measure time taken." << endl
             << "-f: Write similarity values to / read it from file." << endl
             << "-d: Use default values for all options." << endl
             << "Mode flags:" << endl
             << "-R: Cluster based on region average and print quality score." << endl
             << "-C: Cluster based on frame classification and print quality score." << endl
             << "-F: Classify frames and print overlap percentage." << endl;
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

    // Try opening the video and set videoName.
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }
    string videoName = basename(argv[1]);

    // Read provided flag, if given
    if (hasFlag) {
        // Mode flag
        if (last_arg.find('R') != string::npos) {
            qualityMeasurement = true;
            regionClusterMode = true;
            cout << "Clustering mode based on region activated." << endl;
        }
        if (last_arg.find('C') != string::npos) {
            qualityMeasurement = true;
            classificationClusterMode = true;
            cout << "Clustering mode based on frame classification activated." << endl;
        }
        if (last_arg.find('F') != string::npos) {
            qualityMeasurement = true;
            frameClassifyMode = true;
            cout << "Frame classification mode activated." << endl;
        }

        // Additional modifier flags
        if (last_arg.find('v') != string::npos) {
            verbose = true;
            cout << "Verbose output activated." << endl;
        }
        if (last_arg.find('t') != string::npos) {
            measureTime = true;
            cout << "Time taken will be measured." << endl;
        }
        if (last_arg.find('f') != string::npos) {
            fileIO = true;
            cout << "Similarity values will be written to / read from file." << endl;
        }
        if (last_arg.find('d') != string::npos) {
            useDefaults = true;
            cout << "Default values will be used." << endl;
        }
    }

    // Quality mode: Verify that the tag file directory exists and that it contains a folder for the given video.
    string pathToTagFiles = "INVALID";
    if (qualityMeasurement) {
        pathToTagFiles = utils::combine({ defaults::tagFileDirectory, "/", utils::removeExtension(videoName) });

        if (!useDefaults) {
            string userInput = "";
            cout << "Please input the directory which contains tag files "
                 << "(\"d\" for \"" << pathToTagFiles << "\")..." << endl;
            cin >> userInput;

            if (userInput != "d") pathToTagFiles = userInput;
        }

        if (!utils::canOpenDir(pathToTagFiles)) {
            cerr << "Could not open tag file directory \"" << pathToTagFiles << "\"" << endl;
            return 1;
        }
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
            if (subSpecifier < 0 || subSpecifier > 2) {
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

    // File I/O: Verify the working directory can be opened and check if a file for the current video exists.
    string workingDirectory = defaults::workingDirectory;
    string ioFilePath = "INVALID";
    bool fileExists = false;
    if (fileIO) {
        string ioFileName = utils::getCsvFileName(videoName, metricIndex, subSpecifier);
        ioFilePath = workingDirectory + "/" + ioFileName;

        if (!utils::canOpenDir(workingDirectory)) {
            int directoryNotCreated = mkdir(workingDirectory.c_str(), 0777);
            if (directoryNotCreated) {
                cerr << "Could not create or access working directory \"" << workingDirectory << "\"" << endl;
                return 1;
            }

            if (verbose) cout << "Created working directory at \"" << workingDirectory << "\"" << endl;
        } else if (verbose) cout << "Found working directory at \"" << workingDirectory << "\"" << endl;

        // Check to see if a CSV file with the name of the video exists.
        if (!useDefaults) {
            string userInput = "";
            cout << "Please input the file name to write to / read from (\"d\" for \"" << ioFileName << "\")..." << endl;
            cin >> userInput;

            if (userInput != "d") ioFilePath = workingDirectory + "/" + userInput;
        }

        if (!utils::fileExists(ioFilePath)) {
            if (verbose) cout << "No IO file \"" << ioFilePath << "\" found. Will recalculate "
                              << "similarity and save to file." << endl;
            ofstream ioFileStream;
            ioFileStream.open(ioFilePath);
            ioFileStream << "Frame no.,Milliseconds,Similarity to last frame" << endl;
            ioFileStream.close();
        } else {
            fileExists = true;
            if (verbose) cout << "Found IO file \"" << ioFilePath << "\". Will skip "
                              << "similarity calculation and read from file." << endl;
        }
    }

    // Get start time (after user I/O) for time measurement
    time_t startTime = time(nullptr);

    // Don't calculate similarity if file I/O is not active or if the file doesn't exist yet.
    vector<FrameInfo> frames;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    if (!fileIO || !fileExists) {

        // Framewise metric computation
        Mat current, previous;

        // Load first frame
        capture >> previous;
        frames.push_back(FrameInfo(previous, 1, 0));
        if (fileIO) {
            similarityFileUtils::appendToCsv(ioFilePath, 1, 0, -1);
        }
        for (int frameCounter = 2; frameCounter <= totalFrames; frameCounter++) {

            capture >> current;

            if (current.data == NULL) {
                break;
            }

            double currentSimilarity = comparer->computeSimilarity(&previous, &current);
            frames.push_back(FrameInfo(current, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity));
            if (fileIO) {
                similarityFileUtils::appendToCsv(ioFilePath, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity);
            }

            current.copyTo(previous);

            if (verbose) {
                cout << "Similarity " << currentSimilarity
                     << " for frame #" << frameCounter << "/" << totalFrames
                     << " at " << capture.get(CAP_PROP_POS_MSEC) << " msec"
                     << " compared to the last frame" << endl;
            }
        }
    } else {
        // Read in data from file.
        frames = similarityFileUtils::readFrameInfosFromCsv(ioFilePath, totalFrames);
    }

    time_t similarityFinishedTime = time(nullptr);

    if (regionClusterMode) {
        announceMode("Region-average-based clustering");
        clusterRegion(frames, pathToTagFiles, verbose);
    }
    if (classificationClusterMode) {
        announceMode("Frame-based clustering");
        clusterLabels(frames, pathToTagFiles, verbose);
    }
    if (frameClassifyMode) {
        announceMode("Frame classification");
        classify(frames, pathToTagFiles, verbose);
    }

    time_t qualityFinishedTime = time(nullptr);

    if (measureTime) {
        cout << "----------" << endl
             << "Time taken" << endl
             << "Total: " << qualityFinishedTime - startTime << " s" << endl
             << "Similarity measurement: " << similarityFinishedTime - startTime << " s" << endl
             << "Quality measurement: " << qualityFinishedTime - similarityFinishedTime << " s" << endl;
    }

    delete comparer;
    return 0;
}

/**
 * Clusters the frames based on a region average score and evaluates the result with the tag files given.
 * @param frames Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void clusterRegion(vector<FrameInfo> frames, string pathToTagFiles, bool verbose) {
    // 1) Cluster based on region average (refined)
    clusterer::calculateRegionAverage(&frames);
    ClusterInfoContainer clusters = clusterer::cluster(clusterer::AVERAGE_REFINED, frames, verbose);
    // 2) Classify clusters
    classifier::classifyClusters(&clusters);
    // 3) Group and evaluate
    groupAndEvaluate(clusters, pathToTagFiles, verbose);
}

/**
 * Clusters the frames based on single frame classification and evaluates the result with the tag files given.
 * @param frames Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void clusterLabels(vector<FrameInfo> frames, string pathToTagFiles, bool verbose) {
    // 1) Classify frames
    classifier::classifyFramesSingle(&frames);
    // 2) Cluster based on classification
    ClusterInfoContainer clusters = clusterer::cluster(clusterer::LABELS, frames, verbose);
    // 3) Group and evaluate
    groupAndEvaluate(clusters, pathToTagFiles, verbose);
}

void groupAndEvaluate(ClusterInfoContainer clusters, string pathToTagFiles, bool verbose) {
    // Group clustering
    vector<ClusterInfoContainer> groupedClusters = clusterer::group(clusters);

    // Evaluate groups
    vector<tuple<double, string>> results;
    for (ClusterInfoContainer group : groupedClusters) {
        cout << " -- Scoring clustering \"" << group.name << "\"... -- " << endl;
        double score = qualityMeasurer::scoreQuality(pathToTagFiles, group, verbose);
        results.push_back(tuple<double, string>(score, group.name));
    }

    cout << endl;
    for (tuple<double, string> result : results) {
        cout << "Achieved a quality score of " << get<0>(result)
             << " for clustering \"" << get<1>(result) << "\"!" << endl;
    }
}

/**
 * Classifies the frames based on threshold values and compares the result with the tag files given.
 * @param frames Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void classify(vector<FrameInfo> frames, string pathToTagFiles, bool verbose) {
    vector<FrameInfo> highSimilarityFrames;
    vector<FrameInfo> averageSimilarityFrames;
    vector<FrameInfo> lowSimilarityFrames;

    classifier::classifyFramesSingle(&frames);
    for (FrameInfo frame : frames) {
        if (frame.label == classifier::highSimLabel) highSimilarityFrames.push_back(frame);
        else if (frame.label == classifier::mediumSimLabel) averageSimilarityFrames.push_back(frame);
        else lowSimilarityFrames.push_back(frame);
    }

    double totalFrames = frames.size();
    cout << "High similarity:    " << highSimilarityFrames.size() << " of " << totalFrames << " total frames" << endl
         << "Average similarity: " << averageSimilarityFrames.size() << " of " << totalFrames << " total frames" << endl
         << "Low similarity:     " << lowSimilarityFrames.size() << " of " << totalFrames << " total frames" << endl
         << endl;

    cout << "Matching high similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, highSimilarityFrames, verbose);

    cout << "Matching average similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, averageSimilarityFrames, verbose);

    cout << "Matching low similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, lowSimilarityFrames, verbose);
}

/**
 * Outputs announcement about the given mode.
 * @param mode Name of the mode to announce.
 */
void announceMode(string mode) {
    cout << endl << "----------" << endl << "(MODE) " << mode << "..." << endl;
}
