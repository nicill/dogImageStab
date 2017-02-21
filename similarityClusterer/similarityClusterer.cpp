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

using namespace std;
using namespace cv;

// Declarations
void clusterRegion(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose);
void clusterFrame(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose);
void cluster(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose);
void classify(vector<FrameInfo> frames, string pathToTagFiles, bool verbose);
bool canOpenDir(string path);
vector<FrameInfo> readFrameInfosFromCsv(string filePath);
void appendToCsv(string filePath, double frameNo, double msec, double similarity);
double getAverageSimilarity(vector<FrameInfo> cluster);
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
    bool clusterRegionMode = false;
    bool clusterFrameMode = false;
    bool frameMode = false;
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

    // Try opening the video
    VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }

    // Read provided flag, if given
    if (hasFlag) {
        // Mode flag
        if (last_arg.find('R') != string::npos) {
            qualityMeasurement = true;
            clusterRegionMode = true;
            cout << "Clustering mode based on region activated." << endl;
        }
        if (last_arg.find('C') != string::npos) {
            qualityMeasurement = true;
            clusterFrameMode = true;
            cout << "Clustering mode based on frame classification activated." << endl;
        }
        if (last_arg.find('F') != string::npos) {
            qualityMeasurement = true;
            frameMode = true;
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

    // Quality mode: Verify that the tag file directory exists.
    string pathToTagFiles = "INVALID";
    if (qualityMeasurement) {
        pathToTagFiles = defaults::pathToTagFiles;
        if (!useDefaults) {
            string userInput = "";
            cout << "Please input the directory which contains the tag files (\"d\" for \"" << pathToTagFiles << "\")..." << endl;
            cin >> userInput;

            if (userInput != "d") pathToTagFiles = userInput;
        }

        if (!canOpenDir(pathToTagFiles)) {
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

    // File I/O: Verify the working directory can be opened and check if a file for the current video exists.
    string workingDirectory = defaults::workingDirectory;
    string ioFilePath = "INVALID";
    bool fileExists = false;
    if (fileIO) {
        string videoName = basename(argv[1]);
        string ioFileName = videoName + "_" + to_string(metricIndex) + "_" + to_string(subSpecifier) + ".csv";
        ioFilePath = workingDirectory + "/" + ioFileName;

        if (!canOpenDir(workingDirectory)) {
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
        struct stat result;
        int fileNotFound = stat(ioFilePath.c_str(), &result);
        if (fileNotFound) {
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
    vector<FrameInfo> frameInfos;
    double totalFrames = capture.get(CAP_PROP_FRAME_COUNT);
    if (!fileIO || !fileExists) {

        // Framewise metric computation
        Mat current, previous;

        // Load first frame
        capture >> previous;
        frameInfos.push_back(FrameInfo(previous, 1, 0));
        if (fileIO) {
            appendToCsv(ioFilePath, 1, 0, -1);
        }
        for (int frameCounter = 2; frameCounter <= totalFrames; frameCounter++) {

            capture >> current;

            if (current.data == NULL) {
                break;
            }

            double currentSimilarity = comparer->computeSimilarity(&previous, &current);
            frameInfos.push_back(FrameInfo(current, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity));
            if (fileIO) {
                appendToCsv(ioFilePath, frameCounter, capture.get(CAP_PROP_POS_MSEC), currentSimilarity);
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
        frameInfos = readFrameInfosFromCsv(ioFilePath);
        assert(frameInfos.size() == totalFrames);
    }

    time_t similarityFinishedTime = time(nullptr);

    if (clusterRegionMode) {
        announceMode("Region-average-based clustering");
        clusterRegion(frameInfos, pathToTagFiles, verbose);
    }
    if (clusterFrameMode) {
        announceMode("Frame-based clustering");
        clusterFrame(frameInfos, pathToTagFiles, verbose);
    }
    if (frameMode) {
        announceMode("Frame classification");
        classify(frameInfos, pathToTagFiles, verbose);
    }

    time_t qualityFinishedTime = time(nullptr);

    if (measureTime) {
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
 * Clusters the frames based on a region average score and evaluates the result with the tag files given.
 * @param frameInfos Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void clusterRegion(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose) {
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

    cluster(frameInfos, pathToTagFiles, verbose);
}

/**
 * Clusters the frames based on single frame classification and evaluates the result with the tag files given.
 * @param frameInfos Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void clusterFrame(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose) {
    for (int i = 0; i < frameInfos.size(); i++) {
        frameInfos[i].averageSimilarity = frameInfos[i].similarityToPrevious;
    }

    cluster(frameInfos, pathToTagFiles, verbose);
}

/**
 * Clusters frames based on average similarity and evaluates the result with the tag files given.
 * @param frameInfos Frames to cluster.
 * @param pathToTagFiles Directory containing the tag files. Must exist.
 * @param verbose Activate verbosity to cout.
 */
void cluster(vector<FrameInfo> frameInfos, string pathToTagFiles, bool verbose) {
    // Classify frames
    // Cluster
    // TODO cluster based on classification?

    // Find clusters
    vector<ClusterInfo> clusters = { ClusterInfo("All frames", frameInfos) };
    int iteration = 0;
    for (; iteration < 500; iteration++) {
        vector<ClusterInfo> newClusters;

        int currentCluster = 0;

        for (int i = 0; i < clusters.size(); i++) {
            for (int j = 0; j < clusters[i].frames.size(); j++) {
                // Always add first frame of first cluster to allow for comparison.
                if (i == 0 && j == 0) {
                    newClusters.push_back(ClusterInfo(to_string(currentCluster), { clusters[0].frames.front() }));
                    continue;
                }

                // If the difference is too big, we create a new cluster, otherwise we add to the current one.
                if (0.1 < abs(newClusters[currentCluster].averageSimilarity - clusters[i].frames[j].averageSimilarity)) {
                    currentCluster++;
                    newClusters.push_back(ClusterInfo(to_string(currentCluster), { clusters[i].frames[j] }));
                } else {
                    newClusters[currentCluster].addFrameAtBack(clusters[i].frames[j]);
                }
            }
        }

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

    double score = qualityMeasurer::scoreQuality(pathToTagFiles, clusters, verbose);
    cout << "Achieved a quality score of " << score << "!" << endl;
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

    for (FrameInfo frame : frames) {
        if (frame.similarityToPrevious > 0.6) highSimilarityFrames.push_back(frame);
        else if (frame.similarityToPrevious > 0.3) averageSimilarityFrames.push_back(frame);
        else lowSimilarityFrames.push_back(frame);
    }

    double framesSize = frames.size();
    cout << "High similarity:    " << highSimilarityFrames.size() << " of " << framesSize << " total frames" << endl
         << "Average similarity: " << averageSimilarityFrames.size() << " of " << framesSize << " total frames" << endl
         << "Low similarity:     " << lowSimilarityFrames.size() << " of " << framesSize << " total frames" << endl
         << endl;

    cout << "Matching high similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, highSimilarityFrames, verbose);

    cout << "Matching average similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, averageSimilarityFrames, verbose);

    cout << "Matching low similarity frames..." << endl;
    qualityMeasurer::calculateOverlap(pathToTagFiles, lowSimilarityFrames, verbose);
}

/**
 * Checks to see if a given directory can be opened.
 * @param path The directory to check.
 * @return True for success.
 */
bool canOpenDir(string path) {
    DIR *directory = opendir(path.c_str());
    if (directory == nullptr || readdir(directory) == nullptr) {
        return false;
    }

    closedir(directory);
    return true;
}

/**
 * Reads in the given csv file to retrieve the saved frame infos.
 * @param csvStream The file stream. Needs to be open.
 * @return An empty vector if unsuccessful.
 */
vector<FrameInfo> readFrameInfosFromCsv(string filePath) {
    ifstream csvStream;
    csvStream.open(filePath);
    assert(csvStream.is_open());

    vector<FrameInfo> frameInfos;
    string line;
    // Read in header line.
    if (!getline(csvStream, line)) return vector<FrameInfo>();
    while (getline(csvStream, line)) {
        vector<char> charLine(line.c_str(), line.c_str() + line.size() + 1u);

        double frameNo = stod(strtok(&charLine[0], ","));
        double msec = stod(strtok(nullptr, ","));
        double similarity = stod(strtok(nullptr, ","));
        assert(nullptr == strtok(nullptr, ","));

        frameInfos.push_back(FrameInfo(Mat(), frameNo, msec, similarity));
    }

    csvStream.close();
    return frameInfos;
}

/**
 * Appends the given values to the given csv file (must be writable) with highest precision.
 * @param filePath File path of the csv file.
 */
void appendToCsv(string filePath, double frameNo, double msec, double similarity) {
    char sep = ',';
    ofstream ioFileStream;
    // Setting the precision is not strictly necessary, but useful to preserve exact similarity values.
    ioFileStream << setprecision(100);
    ioFileStream.open(filePath, ios::app); // append
    ioFileStream << frameNo << sep << msec << sep << similarity << endl;
    ioFileStream.close();
}

/**
 * Takes the given FrameInfo objects and calculates the average of their average similarity value.
 * @param clusteredInfos FrameInfo objects.
 * @return Average of all average similarity values.
 */
double getAverageSimilarity(std::vector<FrameInfo> clusteredInfos) {
    double summedUpAverages = 0;
    for (FrameInfo frameInfo : clusteredInfos) {
        summedUpAverages += frameInfo.averageSimilarity;
    }
    return summedUpAverages / clusteredInfos.size();
}

/**
 * Outputs announcement about the given mode.
 * @param mode Name of the mode to announce.
 */
void announceMode(string mode) {
    cout << endl << "----------" << endl << "(MODE) " << mode << "..." << endl;
}
