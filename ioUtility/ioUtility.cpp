//
// Created by tokuyama on 17/02/23.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "../similarityClusterer/defaults.h"
#include "../similarityClusterer/clusterer.h"
#include "../similarityClusterer/classifier.h"
#include "../similarityClusterer/similarityFileUtils.cpp"

using std::string;
using std::to_string;
using std::cout;
using std::cerr;
using std::endl;
using std::ofstream;

// Declarations
void help();
int  main(int argc, char **argv);
int  mainCsvMode();
int  mainTagMode();
bool getTagFilesByNames(string, vector<string>, vector<string>, ClusterInfoContainer*, ClusterInfoContainer*);

string workingDirectory = getenv("HOME");
string ioFileName;
string ioFilePath;
string pathToTagFiles;
double totalFrames;

/**
 * Output help.
 */
void help() {
    cout << "Usage: ./name video.mp4 metricIndex subSpecifier -flag" << endl
         << endl
         << "Flags (always supply exactly one)" << endl
         << "-c: Create csv file from data" << endl
         << "-t: Read from / write to tag files" << endl;
}

/**
 * Main method. Allow for some operations, like creating new tag files
 * or outputting data in specific formats
 */
int main(int argc, char **argv) {
    if (argc == 2 && string(argv[1]) == "--help") {
        help();
        return 0;
    }

    // Check for flag
    string last_arg = string(argv[argc - 1]);
    bool hasFlag = last_arg.find("-") == 0;

    // Check validity of arguments
    if (!hasFlag || argc != 5) {
        help();
        return 1;
    }

    int metricIndex = atoi(argv[2]);
    int subSpecifier = atoi(argv[3]);

    // Is the video path given valid?
    cv::VideoCapture capture(argv[1]);
    if (!capture.isOpened()) {
        cerr << "Could not open the video supplied: " << argv[1] << endl;
        return 1;
    }
    totalFrames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    capture.release();
    string videoName = basename(argv[1]);

    // Check validity of working directory and tag file directory
    pathToTagFiles = utils::combine({ defaults::tagFileDirectory, "/", videoName });
    if (!utils::canOpenDir(defaults::workingDirectory) || !utils::canOpenDir(pathToTagFiles)) {
        cerr << "Could not open directory, please check these directories:" << endl
             << defaults::workingDirectory << endl
             << pathToTagFiles << endl;
        return 1;
    }

    // Check existence of IO file.
    ioFileName = utils::getCsvFileName(basename(argv[1]), metricIndex, subSpecifier);
    ioFilePath = utils::combine({ defaults::workingDirectory, "/", ioFileName });
    if (!utils::fileExists(ioFilePath)) {
        cout << "No IO file \"" << ioFilePath << "\" found. Please calculate similarity and save to the given file "
             << "before running this program." << endl;
        return 1;
    }

    // Main logic.
    if (last_arg.find('c') != string::npos) {
        return mainCsvMode();
    } else if (last_arg.find('t') != string::npos) {
        return mainTagMode();
    }
}

int mainCsvMode() {
    cout << "CSV mode activated" << endl
         << "- Using working directory \"" << workingDirectory << "\"" << endl
         << "- Reading similarity values from directory \"" << defaults::workingDirectory << "\"" << endl
         << "- Reading tag files from directory \"" << pathToTagFiles << "\"" << endl << endl;

    // 1) Read tag files and make sure we have tag files for bark and stop.
    ClusterInfoContainer barkFileClusters;
    ClusterInfoContainer stopFileClusters;
    bool success = getTagFilesByNames(pathToTagFiles,
                                      {"bark", "Bark"}, {"stop", "Stop"},
                                      &barkFileClusters, &stopFileClusters);

    if (!success) {
        return 1;
    }

    // 2) Read similarity from video file, build clusters and label
    vector<FrameInfo> frames = similarityFileUtils::readFrameInfosFromCsv(ioFilePath, totalFrames);
    //    Steps: Calculate region average, cluster, classify clusters
    clusterer::calculateRegionAverage(&frames);
    ClusterInfoContainer clusters = clusterer::cluster(clusterer::AVERAGE_REFINED, frames, false);
    classifier::classifyClusters(&clusters);

    vector<FrameInfo>::iterator framesIterator = frames.begin();
    for (const auto &cluster : clusters.clusterInfos) {
        for (const auto &frame : cluster.frames) {
            assert((*framesIterator).frameNo == frame.frameNo);

            if (cluster.label == classifier::lowSimLabel) {
                (*framesIterator).label = "1";
            } else if (cluster.label == classifier::mediumSimLabel) {
                (*framesIterator).label = "2";
            } else if (cluster.label == classifier::highSimLabel) {
                (*framesIterator).label = "3";
            } else {
                throw ("Label is unknown");
            }

            framesIterator++;
        }
    }

    // 3) Write result to CSV file
    //    Frame, Msec, Similarity, Avg. similarity, Classification, Stop, Bark
    //    1,     0,    0.321,      0.6663333211     1,              1,    1
    //    2,     200,  0.9833,     0.8877472988     3,              0,    1
    //    ...
    ofstream fileStream;
    string filePath = workingDirectory + "/" + ioFileName + "_extended.csv";
    if(utils::fileExists(filePath)) {
        cerr << "Please make sure the file \"" << filePath << "\" that's supposed to be written to doesn't exist" << endl;
        return 1;
    }

    fileStream.open(filePath);
    fileStream << "Frame,Msec,Similarity,Avg. similarity,Classification,Stop,Bark" << endl;
    fileStream.close();

    // Iterate through all FrameInfo objects and save info plus stop and bark field
    vector<ClusterInfo>::iterator barkFileClustersIterator = barkFileClusters.clusterInfos.begin();
    vector<ClusterInfo>::iterator stopFileClustersIterator = stopFileClusters.clusterInfos.begin();
    for (FrameInfo frame : frames) {
        while (frame.msec > (*barkFileClustersIterator).endMsec
               && barkFileClustersIterator != barkFileClusters.clusterInfos.end()) {
            barkFileClustersIterator++;
        }
        while (frame.msec > (*stopFileClustersIterator).endMsec
               && stopFileClustersIterator != stopFileClusters.clusterInfos.end()) {
            stopFileClustersIterator++;
        }

        bool stop = (*stopFileClustersIterator).containsFrame(frame);
        bool bark = (*barkFileClustersIterator).containsFrame(frame);

        similarityFileUtils::appendToCsv(filePath, frame.frameNo, frame.msec, frame.similarityToPrevious,
                                         frame.averageSimilarity, frame.label, stop, bark);
    }

    cout << "File \"" << filePath << "\" has been written successfully." << endl;
    return 0;
}

int mainTagMode() {
    cout << "Tag file mode activated" << endl;

    // 1. Load tag files to be combined and make sure we have tag files for bark and stop.
    ClusterInfoContainer barkFileClusters;
    ClusterInfoContainer stopFileClusters;
    bool success = getTagFilesByNames(pathToTagFiles,
                                      {"bark", "Bark"}, {"stop", "Stop"},
                                      &barkFileClusters, &stopFileClusters);

    if (!success) {
        return 1;
    }
    // 2. Iterate through clusters and combine with AND
    ClusterInfoContainer overlaps = ClusterInfoContainer("Overlaps of bark and stop");
    vector<ClusterInfo>::iterator barkFileClustersIterator = barkFileClusters.clusterInfos.begin();
    vector<ClusterInfo>::iterator stopFileClustersIterator = stopFileClusters.clusterInfos.begin();
    while (barkFileClustersIterator != barkFileClusters.clusterInfos.end()
           && stopFileClustersIterator != stopFileClusters.clusterInfos.end()) {
        if ((*barkFileClustersIterator).beginMsec > (*stopFileClustersIterator).endMsec) {
            barkFileClustersIterator++;
            continue;
        }

        if ((*stopFileClustersIterator).beginMsec > (*barkFileClustersIterator).endMsec) {
            stopFileClustersIterator++;
            continue;
        }


    }

    // TODO implement
    throw("NOT IMPLEMENTED");
}

/**
 * Checks the given list for two ClusterInfoContainer objects with one of the respective, given names.
 */
bool getTagFilesByNames(string pathToTagFiles,
                        vector<string> names_1, vector<string> names_2,
                        ClusterInfoContainer *fileClusters_1, ClusterInfoContainer *fileClusters_2) {
    vector<ClusterInfoContainer> allTagFileClusters = similarityFileUtils::readTagFiles(pathToTagFiles, false);

    int hasBoth = 0;
    for (ClusterInfoContainer tagFile : allTagFileClusters) {
        if (utils::stringContainsAny(tagFile.name, names_1)) {
            hasBoth |= 1;
            (*fileClusters_1) = tagFile;
        } else if (utils::stringContainsAny(tagFile.name, names_2)) {
            hasBoth |= 2;
            (*fileClusters_2) = tagFile;
        }
    }

    if ((hasBoth & 3) != 3) {
        cerr << "Couldn't find tag files for "
             << utils::combine(names_1, ", ") << " and/or "
             << utils::combine(names_2, ", ") << ". Please make sure both exist and are named correctly."
             << endl;
        return false;
    }

    return true;
}
