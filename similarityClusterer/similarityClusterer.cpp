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

using namespace std;
using namespace cv;

// Declarations
bool canOpenDir(string path);
vector<FrameInfo> readFrameInfosFromCsv(string filePath);
void appendToCsv(string filePath, double frameNo, double msec, double similarity);
double getClusterAverage(vector<FrameInfo> cluster);

int main(int argc, char **argv) {
    bool verbose = false;
    bool qualityMode = false;
    bool timeMode = false;
    bool fileIO = false;
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
             << "-f: Write similarity values to / read it from file." << endl
             << "-d: Use default values for all options." << endl;
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
    if (qualityMode) {
        pathToTagFiles = "/home/tokuyama/dog/tags";
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
    string workingDirectory = "/home/tokuyama/similarityClusterer_files";
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
        }
        else if (verbose) cout << "Found working directory at \"" << workingDirectory << "\"" << endl;

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

    // Get start time (after user I/O) for timeMode
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

        current.~Mat();
        previous.~Mat();
    } else {
        // Read in data from file.
        frameInfos = readFrameInfosFromCsv(ioFilePath);
        assert(frameInfos.size() == totalFrames);
    }
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

    time_t similarityFinishedTime = time(nullptr);

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
    }

    double score = 0;
    if (qualityMode) {
        score = qualityMeasurer::scoreQuality(pathToTagFiles, clusters, verbose);
        cout << "Achieved a quality score of " << score << " for the video \"" << argv[1] << "\"!" << endl;
    }

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
double getClusterAverage(vector<FrameInfo> clusteredInfos) {
    double summedUpAverages = 0;
    for (FrameInfo frameInfo : clusteredInfos) {
        summedUpAverages += frameInfo.averageSimilarity;
    }
    return summedUpAverages / clusteredInfos.size();
}
